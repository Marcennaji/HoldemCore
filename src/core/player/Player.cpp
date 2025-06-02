/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU Affero General Public License as            *
 * published by the Free Software Foundation, either version 3 of the        *
 * License, or (at your option) any later version.                           *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU Affero General Public License for more details.                       *
 *                                                                           *
 * You should have received a copy of the GNU Affero General Public License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#include "Player.h"

#include <core/engine/CardsValue.h>
#include <core/engine/Randomizer.h>
#include <core/engine/model/Ranges.h>
#include <core/interfaces/IHand.h>
#include <core/interfaces/ILogger.h>
#include <core/interfaces/persistence/IHandAuditStore.h>
#include <core/interfaces/persistence/IPlayersStatisticsStore.h>
#include <third_party/psim/psim.hpp>
#include "Helpers.h"

namespace pkt::core
{

using namespace std;

void CurrentHandActions::reset()
{

    m_preflopActions.clear();
    m_flopActions.clear();
    m_turnActions.clear();
    m_riverActions.clear();
}

std::vector<PlayerAction>& CurrentHandActions::getPreflopActions()
{
    return m_preflopActions;
}

std::vector<PlayerAction>& CurrentHandActions::getFlopActions()
{
    return m_flopActions;
}

std::vector<PlayerAction>& CurrentHandActions::getTurnActions()
{
    return m_turnActions;
}

std::vector<PlayerAction>& CurrentHandActions::getRiverActions()
{
    return m_riverActions;
}

Player::Player(GameEvents* events, IHandAuditStore* ha, IPlayersStatisticsStore* ps, int id, PlayerType type,
               std::string name, int sC, bool aS, int mB)
    : myHandAuditStore(ha), myPlayersStatisticsStore(ps), currentHand(0), myID(id), myType(type), myName(name),
      myCardsValueInt(0), myCash(sC), mySet(0), myLastRelativeSet(0), myAction(PLAYER_ACTION_NONE), myButton(mB),
      myActiveStatus(aS), myTurn(0), myCardsFlip(0), myRoundStartCash(0), lastMoneyWon(0), m_isSessionActive(false),
      myEvents(events)
{
    loadStatistics();

    int i;
    for (i = 0; i < 2; i++)
    {
        myCards[i] = -1;
    }

    for (i = 0; i < 5; i++)
    {
        myBestHandPosition[i] = -1;
    }
}

void Player::setHand(IHand* br)
{
    currentHand = br;
}

void Player::setIsSessionActive(bool active)
{
    m_isSessionActive = active;
}

bool Player::isSessionActive() const
{
    return m_isSessionActive;
}

const PlayerPosition Player::getPosition() const
{
    return myPosition;
}

void Player::setPosition()
{

    myPosition = UNKNOWN;

    const int dealerPosition = currentHand->getDealerPosition();
    const int nbPlayers = currentHand->getActivePlayerList()->size();

    // first dimension is my relative position, after the dealer.
    // Second dimension is the corrresponding position, depending on the number of players (from 0 to 10)
    const PlayerPosition onDealerPositionPlus[MAX_NUMBER_OF_PLAYERS][MAX_NUMBER_OF_PLAYERS + 1] = {
        // 0 player	1 player	2 players	3 players	4 players		5 players		6 players		7 players 8
        // players		9 players		10 players
        {UNKNOWN, UNKNOWN, BUTTON, BUTTON, BUTTON, BUTTON, BUTTON, BUTTON, BUTTON, BUTTON,
         BUTTON},                                                                   // my position = dealer
        {UNKNOWN, UNKNOWN, BB, SB, SB, SB, SB, SB, SB, SB, SB},                     // my position = dealer + 1
        {UNKNOWN, UNKNOWN, UNKNOWN, BB, BB, BB, BB, BB, BB, BB, BB},                // my position = dealer + 2
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, CUTOFF, UTG, UTG, UTG, UTG, UTG, UTG}, // my position = dealer + 3
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, CUTOFF, MIDDLE, MIDDLE, UTG_PLUS_ONE, UTG_PLUS_ONE,
         UTG_PLUS_ONE}, // my position = dealer + 4
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, CUTOFF, LATE, MIDDLE, UTG_PLUS_TWO,
         UTG_PLUS_TWO}, // my position = dealer + 5
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, CUTOFF, LATE, MIDDLE,
         MIDDLE}, // my position = dealer + 6
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, CUTOFF, LATE,
         MIDDLE_PLUS_ONE}, // my position = dealer + 7
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, CUTOFF,
         LATE}, // my position = dealer + 8
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
         CUTOFF} // my position = dealer + 9
    };

    // first dimension is my relative position, BEHIND the dealer.
    // Second are the corrresponding positions, depending on the number of players
    const PlayerPosition onDealerPositionMinus[10][11] = {
        // 0 player	1 player	2 players	3 players	4 players		5 players		6 players		7 players 8
        // players		9 players		10 players
        {UNKNOWN, UNKNOWN, BUTTON, BUTTON, BUTTON, BUTTON, BUTTON, BUTTON, BUTTON, BUTTON,
         BUTTON},                                                                           // my position = dealer
        {UNKNOWN, UNKNOWN, BB, BB, CUTOFF, CUTOFF, CUTOFF, CUTOFF, CUTOFF, CUTOFF, CUTOFF}, // my position = dealer - 1
        {UNKNOWN, UNKNOWN, UNKNOWN, SB, BB, UTG, MIDDLE, LATE, LATE, LATE, LATE},           // my position = dealer - 2
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, SB, BB, UTG, MIDDLE, MIDDLE, MIDDLE_PLUS_ONE,
         MIDDLE_PLUS_ONE}, // my position = dealer - 3
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, SB, BB, UTG, UTG_PLUS_ONE, UTG_PLUS_TWO,
         MIDDLE}, // my position = dealer - 4
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, SB, BB, UTG, UTG_PLUS_ONE,
         UTG_PLUS_TWO}, // my position = dealer - 5
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, SB, BB, UTG,
         UTG_PLUS_ONE}, // my position = dealer - 6
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, SB, BB,
         UTG}, // my position = dealer - 7
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, SB,
         BB}, // my position = dealer - 8
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
         SB} // my position = dealer - 9

    };

    if (myID == dealerPosition)
        myPosition = BUTTON;
    else
    {

        // get my relative position from the dealer
        PlayerListIterator it_c;
        PlayerList players = currentHand->getActivePlayerList();

        int pos = 0;

        if (myID > dealerPosition)
        {

            bool dealerFound = false;
            for (it_c = players->begin(); it_c != players->end(); ++it_c)
            {
                if ((*it_c)->getID() == dealerPosition)
                    dealerFound = true;
                else if (dealerFound)
                    pos++;
                if ((*it_c)->getID() == myID)
                    break;
            }
            myPosition = onDealerPositionPlus[pos][nbPlayers];
        }
        else
        {
            // myId < dealerPosition
            bool myPositionFound = false;
            for (it_c = players->begin(); it_c != players->end(); ++it_c)
            {

                if ((*it_c)->getID() == myID)
                    myPositionFound = true;
                else if (myPositionFound)
                    pos++;
                if ((*it_c)->getID() == dealerPosition)
                    break;
            }
            myPosition = onDealerPositionMinus[pos][nbPlayers];
        }
    }

    assert(myPosition != UNKNOWN);
}
std::string Player::getPositionLabel(PlayerPosition p) const
{

    switch (p)
    {

    case UTG:
        return "UTG";
        break;
    case UTG_PLUS_ONE:
        return "UTG_PLUS_ONE";
        break;
    case UTG_PLUS_TWO:
        return "UTG_PLUS_TWO";
        break;
    case MIDDLE:
        return "MIDDLE";
        break;
    case MIDDLE_PLUS_ONE:
        return "MIDDLE_PLUS_ONE";
        break;
    case LATE:
        return "LATE";
        break;
    case CUTOFF:
        return "CUTOFF";
        break;
    case BUTTON:
        return "BUTTON";
        break;
    case SB:
        return "SB";
        break;
    case BB:
        return "BB";
        break;
    default:
        return "unknown";
        break;
    }
}

int Player::getID() const
{
    return myID;
}
CurrentHandActions& Player::getCurrentHandActions()
{
    return myCurrentHandActions;
}

void Player::setID(unsigned newId)
{
    myID = newId;
}

PlayerType Player::getType() const
{
    return myType;
}

void Player::setName(const std::string& theValue)
{
    myName = theValue;
}
std::string Player::getName() const
{
    return myName;
}

void Player::setCash(int theValue)
{
    myCash = theValue;
}
int Player::getCash() const
{
    return myCash;
}

void Player::setSet(int theValue)
{
    myLastRelativeSet = theValue;
    mySet += theValue;
    myCash -= theValue;
}

void Player::setSetNull()
{
    mySet = 0;
    myLastRelativeSet = 0;
}
int Player::getSet() const
{
    return mySet;
}
int Player::getLastRelativeSet() const
{
    return myLastRelativeSet;
}

void Player::setAction(PlayerAction theValue, bool blind)
{
    myAction = theValue;
}
PlayerAction Player::getAction() const
{
    return myAction;
}

void Player::setButton(int theValue)
{
    myButton = theValue;
}
int Player::getButton() const
{
    return myButton;
}

void Player::setActiveStatus(bool theValue)
{
    myActiveStatus = theValue;
}
bool Player::getActiveStatus() const
{
    return myActiveStatus;
}

void Player::setCards(int* theValue)
{

    for (int i = 0; i < 2; i++)
        myCards[i] = theValue[i];

    // will contain human-readable string, i.e "Qc" or "Ts"
    myCard1 = CardsValue::CardStringValue[myCards[0]];
    myCard2 = CardsValue::CardStringValue[myCards[1]];
}
void Player::getCards(int* theValue) const
{
    int i;
    for (i = 0; i < 2; i++)
        theValue[i] = myCards[i];
}

void Player::setTurn(bool theValue)
{
    myTurn = theValue;
}
bool Player::getTurn() const
{
    return myTurn;
}

void Player::setCardsFlip(bool theValue)
{
    myCardsFlip = theValue;
}
bool Player::getCardsFlip() const
{
    return myCardsFlip;
}

void Player::setCardsValueInt(int theValue)
{
    myCardsValueInt = theValue;
}
int Player::getCardsValueInt() const
{
    return myCardsValueInt;
}

void Player::setBestHandPosition(int* theValue)
{
    for (int i = 0; i < 5; i++)
        myBestHandPosition[i] = theValue[i];
}
void Player::getBestHandPosition(int* theValue) const
{
    for (int i = 0; i < 5; i++)
        theValue[i] = myBestHandPosition[i];
}

void Player::setRoundStartCash(int theValue)
{
    myRoundStartCash = theValue;
}
int Player::getRoundStartCash() const
{
    return myRoundStartCash;
}

void Player::setLastMoneyWon(int theValue)
{
    lastMoneyWon = theValue;
}
int Player::getLastMoneyWon() const
{
    return lastMoneyWon;
}

std::string Player::getCardsValueString() const
{
    std::string s = myCard1 + " " + myCard2;
    return s;
}

const PlayerStatistics& Player::getStatistics(const int nbPlayers) const
{
    return myStatistics[nbPlayers];
}
void Player::resetPlayerStatistics()
{

    for (int i = 0; i <= MAX_NUMBER_OF_PLAYERS; i++)
        myStatistics[i].reset();
}

void Player::updatePreflopStatistics()
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();

    if (myCurrentHandActions.m_preflopActions.size() == 1)
    {
        myStatistics[nbPlayers].m_toTalHands++;
        myStatistics[nbPlayers].m_preflopStatistics.m_hands++;
    }

    switch (myAction)
    {
    case PLAYER_ACTION_ALLIN:
        myStatistics[nbPlayers].m_preflopStatistics.m_raises++;
        break;
    case PLAYER_ACTION_RAISE:
        myStatistics[nbPlayers].m_preflopStatistics.m_raises++;
        break;
    case PLAYER_ACTION_FOLD:
        myStatistics[nbPlayers].m_preflopStatistics.m_folds++;
        break;
    case PLAYER_ACTION_CHECK:
        myStatistics[nbPlayers].m_preflopStatistics.m_checks++;
        break;
    case PLAYER_ACTION_CALL:
        myStatistics[nbPlayers].m_preflopStatistics.m_calls++;
        break;
    default:
        break;
    }

    myStatistics[nbPlayers].m_preflopStatistics.AddLastAction(myAction); // keep track of the last 10 actions

    if (myAction == PLAYER_ACTION_CALL && currentHand->getRaisersPositions().size() == 0) //
        myStatistics[nbPlayers].m_preflopStatistics.m_limps++;

    int playerRaises = 0;
    for (std::vector<PlayerAction>::const_iterator i = myCurrentHandActions.m_preflopActions.begin();
         i != myCurrentHandActions.m_preflopActions.end(); i++)
    {
        if (*i == PLAYER_ACTION_RAISE || *i == PLAYER_ACTION_ALLIN)
            playerRaises++;
    }

    if (myAction == PLAYER_ACTION_RAISE || myAction == PLAYER_ACTION_ALLIN)
    {

        if (playerRaises == 1 && currentHand->getRaisersPositions().size() == 2)
            myStatistics[nbPlayers].m_preflopStatistics.m_3Bets++;

        if (playerRaises == 2 || (playerRaises == 1 && currentHand->getRaisersPositions().size() == 3))
            myStatistics[nbPlayers].m_preflopStatistics.m_4Bets++;
    }
    else
    {

        if (playerRaises == 1)
        {

            myStatistics[nbPlayers].m_preflopStatistics.m_call3BetsOpportunities++;

            if (myAction == PLAYER_ACTION_CALL)
                myStatistics[nbPlayers].m_preflopStatistics.m_call3Bets++;
        }
    }
}
void Player::updateFlopStatistics()
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();

    if (myCurrentHandActions.m_flopActions.size() == 1)
        myStatistics[nbPlayers].m_flopStatistics.m_hands++;

    switch (myAction)
    {
    case PLAYER_ACTION_ALLIN:
        myStatistics[nbPlayers].m_flopStatistics.m_raises++;
        break;
    case PLAYER_ACTION_RAISE:
        myStatistics[nbPlayers].m_flopStatistics.m_raises++;
        break;
    case PLAYER_ACTION_FOLD:
        myStatistics[nbPlayers].m_flopStatistics.m_folds++;
        break;
    case PLAYER_ACTION_CHECK:
        myStatistics[nbPlayers].m_flopStatistics.m_checks++;
        break;
    case PLAYER_ACTION_CALL:
        myStatistics[nbPlayers].m_flopStatistics.m_calls++;
        break;
    case PLAYER_ACTION_BET:
        myStatistics[nbPlayers].m_flopStatistics.m_bets++;
        break;
    default:
        break;
    }
    if (myAction == PLAYER_ACTION_RAISE && currentHand->getRaisersPositions().size() > 1)
        myStatistics[nbPlayers].m_flopStatistics.m_3Bets++;

    // continuation bets
    if (currentHand->getPreflopLastRaiserID() == myID)
    {
        myStatistics[nbPlayers].m_flopStatistics.m_continuationBetsOpportunities++;
        if (myAction == PLAYER_ACTION_BET)
            myStatistics[nbPlayers].m_flopStatistics.m_continuationBets++;
    }
}
void Player::updateTurnStatistics()
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();

    if (myCurrentHandActions.m_turnActions.size() == 1)
        myStatistics[nbPlayers].m_turnStatistics.m_hands++;

    switch (myAction)
    {
    case PLAYER_ACTION_ALLIN:
        myStatistics[nbPlayers].m_turnStatistics.m_raises++;
        break;
    case PLAYER_ACTION_RAISE:
        myStatistics[nbPlayers].m_turnStatistics.m_raises++;
        break;
    case PLAYER_ACTION_FOLD:
        myStatistics[nbPlayers].m_turnStatistics.m_folds++;
        break;
    case PLAYER_ACTION_CHECK:
        myStatistics[nbPlayers].m_turnStatistics.m_checks++;
        break;
    case PLAYER_ACTION_CALL:
        myStatistics[nbPlayers].m_turnStatistics.m_calls++;
        break;
    case PLAYER_ACTION_BET:
        myStatistics[nbPlayers].m_turnStatistics.m_bets++;
        break;
    default:
        break;
    }
    if (myAction == PLAYER_ACTION_RAISE && currentHand->getRaisersPositions().size() > 1)
        myStatistics[nbPlayers].m_turnStatistics.m_3Bets++;
}
void Player::updateRiverStatistics()
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();

    if (myCurrentHandActions.m_riverActions.size() == 1)
        myStatistics[nbPlayers].m_riverStatistics.m_hands++;

    switch (myAction)
    {
    case PLAYER_ACTION_ALLIN:
        myStatistics[nbPlayers].m_riverStatistics.m_raises++;
        break;
    case PLAYER_ACTION_RAISE:
        myStatistics[nbPlayers].m_riverStatistics.m_raises++;
        break;
    case PLAYER_ACTION_FOLD:
        myStatistics[nbPlayers].m_riverStatistics.m_folds++;
        break;
    case PLAYER_ACTION_CHECK:
        myStatistics[nbPlayers].m_riverStatistics.m_checks++;
        break;
    case PLAYER_ACTION_CALL:
        myStatistics[nbPlayers].m_riverStatistics.m_calls++;
        break;
    case PLAYER_ACTION_BET:
        myStatistics[nbPlayers].m_riverStatistics.m_bets++;
        break;
    default:
        break;
    }
    if (myAction == PLAYER_ACTION_RAISE && currentHand->getRaisersPositions().size() > 1)
        myStatistics[nbPlayers].m_riverStatistics.m_3Bets++;
}

void Player::loadStatistics()
{

    resetPlayerStatistics(); // reset stats to 0
    myStatistics = myPlayersStatisticsStore->getPlayerStatistics(myName);
}

const PostFlopState Player::getPostFlopState() const
{

    std::string stringHand = getCardsValueString();
    std::string stringBoard = getStringBoard();

    PostFlopState r;

    GetHandState((stringHand + stringBoard).c_str(), &r);

    return r;
}
bool Player::checkIfINeedToShowCards() const
{
    std::list<unsigned> playerNeedToShowCardsList = currentHand->getBoard()->getPlayerNeedToShowCards();
    for (std::list<unsigned>::iterator it = playerNeedToShowCardsList.begin(); it != playerNeedToShowCardsList.end();
         ++it)
    {
        if (*it == myID)
            return true;
    }

    return false;
}
std::string Player::getStringBoard() const
{

    int cardsOnBoard;

    if (currentHand->getCurrentRound() == GAME_STATE_FLOP)
        cardsOnBoard = 3;
    else if (currentHand->getCurrentRound() == GAME_STATE_TURN)
        cardsOnBoard = 4;
    else if (currentHand->getCurrentRound() == GAME_STATE_RIVER)
        cardsOnBoard = 5;
    else
        cardsOnBoard = 0;

    std::string stringBoard;
    int board[5];
    currentHand->getBoard()->getCards(board);

    for (int i = 0; i < cardsOnBoard; i++)
    {
        stringBoard += " ";
        stringBoard += CardsValue::CardStringValue[board[i]];
    }

    return stringBoard;
}

bool Player::getHavePosition(PlayerPosition myPos, PlayerList runningPlayers)
{
    // return true if myPos is last to play, false if not

    bool havePosition = true;

    PlayerListConstIterator it_c;

    for (it_c = runningPlayers->begin(); it_c != runningPlayers->end(); ++it_c)
    {

        if ((*it_c)->getPosition() > myPos)
            havePosition = false;
    }

    return havePosition;
}

std::shared_ptr<Player> Player::getPlayerByUniqueId(unsigned id) const
{
    std::shared_ptr<Player> tmpPlayer;
    PlayerListIterator i = currentHand->getSeatsList()->begin();
    PlayerListIterator end = currentHand->getSeatsList()->end();
    while (i != end)
    {
        if ((*i)->getID() == id)
        {
            tmpPlayer = *i;
            break;
        }
        ++i;
    }
    return tmpPlayer;
}

// convert a range into a  list of real cards
std::vector<std::string> Player::getRangeAtomicValues(std::string ranges, const bool returnRange) const
{

    vector<std::string> result;

    std::istringstream oss(ranges);
    std::string token;

    while (getline(oss, token, ','))
    {

        if (token.size() == 0)
            continue;

        if (token.size() == 1 || token.size() > 4)
        {
            std::cout << "getRangeAtomicValues invalid range : " << token << endl;
            return result;
        }

        const char* range = token.c_str();

        if (token.size() == 2)
        { // an exact pair, like 55 or AA

            string s1;
            s1 = range[0];
            string s2;
            s2 = range[1];

            if (!returnRange)
            {
                result.push_back(s1 + 's' + s2 + 'd');
                result.push_back(s1 + 's' + s2 + 'h');
                result.push_back(s1 + 's' + s2 + 'c');
                result.push_back(s1 + 'd' + s2 + 'h');
                result.push_back(s1 + 'd' + s2 + 'c');
                result.push_back(s1 + 'c' + s2 + 'h');
            }
            else
                result.push_back(s1 + s2);

            continue;
        }

        if (token.size() == 3)
        {

            if (range[0] != range[1] && range[2] == 's')
            { // range is an exact suited hand, like QJs

                string s1;
                s1 = range[0];
                string s2;
                s2 = range[1];

                if (!returnRange)
                {
                    result.push_back(s1 + 's' + s2 + 's');
                    result.push_back(s1 + 'd' + s2 + 'd');
                    result.push_back(s1 + 'h' + s2 + 'h');
                    result.push_back(s1 + 'c' + s2 + 'c');
                }
                else
                    result.push_back(s1 + s2 + 's');

                continue;
            }

            if (range[0] != range[1] && range[2] == 'o')
            { // range is an exact offsuited cards, like KTo

                string s1;
                s1 = range[0];
                string s2;
                s2 = range[1];

                if (!returnRange)
                {
                    result.push_back(s1 + 's' + s2 + 'd');
                    result.push_back(s1 + 's' + s2 + 'c');
                    result.push_back(s1 + 's' + s2 + 'h');

                    result.push_back(s1 + 'd' + s2 + 's');
                    result.push_back(s1 + 'd' + s2 + 'c');
                    result.push_back(s1 + 'd' + s2 + 'h');

                    result.push_back(s1 + 'h' + s2 + 'd');
                    result.push_back(s1 + 'h' + s2 + 'c');
                    result.push_back(s1 + 'h' + s2 + 's');

                    result.push_back(s1 + 'c' + s2 + 'd');
                    result.push_back(s1 + 'c' + s2 + 's');
                    result.push_back(s1 + 'c' + s2 + 'h');
                }
                else
                    result.push_back(s1 + s2 + 'o');

                continue;
            }

            if (range[0] == range[1] && range[2] == '+')
            { // range is a pair and above, like 99+
                char c = range[0];

                while (c != 'X')
                {

                    string s;
                    s = c;

                    if (!returnRange)
                    {
                        result.push_back(s + 's' + s + 'd');
                        result.push_back(s + 's' + s + 'c');
                        result.push_back(s + 's' + s + 'h');
                        result.push_back(s + 'd' + s + 'c');
                        result.push_back(s + 'd' + s + 'h');
                        result.push_back(s + 'h' + s + 'c');
                    }
                    else
                        result.push_back(s + s);

                    // next value :
                    c = incrementCardValue(c);
                }
                continue;
            }
        }

        if (token.size() == 4)
        {

            if (range[0] != range[1] && range[2] == 'o' && range[3] == '+')
            {

                // range is offsuited and above, like AQo+

                string s1;
                s1 = range[0];
                char c = range[1];

                while (c != range[0])
                {

                    string s2;
                    s2 = c;

                    if (!returnRange)
                    {
                        result.push_back(s1 + 's' + s2 + 'd');
                        result.push_back(s1 + 's' + s2 + 'c');
                        result.push_back(s1 + 's' + s2 + 'h');

                        result.push_back(s1 + 'd' + s2 + 's');
                        result.push_back(s1 + 'd' + s2 + 'c');
                        result.push_back(s1 + 'd' + s2 + 'h');

                        result.push_back(s1 + 'h' + s2 + 'd');
                        result.push_back(s1 + 'h' + s2 + 'c');
                        result.push_back(s1 + 'h' + s2 + 's');

                        result.push_back(s1 + 'c' + s2 + 'd');
                        result.push_back(s1 + 'c' + s2 + 's');
                        result.push_back(s1 + 'c' + s2 + 'h');
                    }
                    else
                        result.push_back(s1 + s2 + 'o');

                    // next value :
                    c = incrementCardValue(c);
                }
                continue;
            }
            if (range[0] != range[1] && range[2] == 's' && range[3] == '+')
            {

                // range is suited and above, like AQs+

                string s1;
                s1 = range[0];
                char c = range[1];

                while (c != range[0])
                {

                    string s2;
                    s2 = c;

                    if (!returnRange)
                    {
                        result.push_back(s1 + 's' + s2 + 's');
                        result.push_back(s1 + 'd' + s2 + 'd');
                        result.push_back(s1 + 'h' + s2 + 'h');
                        result.push_back(s1 + 'c' + s2 + 'c');
                    }
                    else
                        result.push_back(s1 + s2 + 's');

                    // next value :
                    c = incrementCardValue(c);
                }
                continue;
            }

            // if not a "suited/unsuited and above" range : range is a real hand, like Ad2c	or JhJd

            if (range[0] == range[2])
            {

                // it's a pair, like JhJd
                if (!returnRange)
                {
                    string s;
                    s += range[0];
                    s += range[0];
                    result.push_back(s);
                }
                else
                    result.push_back(token);
            }

            // real hand but not a pair (like Ad2c) : don't modify it
            result.push_back(token);
        }
    }

    return result;
}

char Player::incrementCardValue(char c) const
{

    switch (c)
    {
    case '2':
        return '3';
    case '3':
        return '4';
    case '4':
        return '5';
    case '5':
        return '6';
    case '6':
        return '7';
    case '7':
        return '8';
    case '8':
        return '9';
    case '9':
        return 'T';
    case 'T':
        return 'J';
    case 'J':
        return 'Q';
    case 'Q':
        return 'K';
    case 'K':
        return 'A';
    default:
        return 'X';
    }
}

int Player::getPotOdd() const
{

    const int highestSet = min(myCash, currentHand->getCurrentBettingRound()->getHighestSet());

    int pot = currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets();

    if (pot == 0)
    { // shouldn't happen, but...
#ifdef LOG_POKER_EXEC
        cout << endl
             << "Error : Pot = " << currentHand->getBoard()->getPot() << " + " << currentHand->getBoard()->getSets()
             << " = " << pot << endl;
#endif
        return 0;
    }

    int odd = (highestSet - mySet) * 100 / pot;
    if (odd < 0)
        odd = -odd; // happens if mySet > highestSet

#ifdef LOG_POKER_EXEC
    // cout << endl << "Pot odd computing : highest set = min(" << myCash << ", " <<
    // currentHand->getCurrentBettingRound()->getHighestSet() << ") = " << highestSet
    //		<< ", mySet = " << mySet
    //		<< ", pot = " << currentHand->getBoard()->getPot() << " + " << currentHand->getBoard()->getSets() << " = "
    //<< pot
    //		<< ", pot odd = (" << highestSet << " - " << mySet << ") * 100 / " << pot << " = " << odd << endl;
#endif

    return odd;
}

float Player::getM() const
{

    int blinds = currentHand->getSmallBlind() + (currentHand->getSmallBlind() * 2); // assume for now that BB is 2 * SB
    if (blinds > 0 && myCash > 0)
        return (float) myCash / blinds;
    else
        return 0;
}

const SimResults Player::getHandSimulation() const
{

    SimResults r;
    const string cards = (getCardsValueString() + getStringBoard()).c_str();

    SimulateHand(cards.c_str(), &r, 0, 1, 0);

    float win = r.win; // save the value

    const int nbOpponents = std::max(1, (int) currentHand->getRunningPlayerList()->size() -
                                            1); // note that allin opponents are not "running" any more
    SimulateHandMulti(cards.c_str(), &r, 200, 100, nbOpponents);
    r.win = win; // because SimulateHandMulti doesn't compute 'win'

    // evaluate my strength against my opponents's guessed ranges :
    float maxOpponentsStrengths = getMaxOpponentsStrengths();

    r.winRanged = 1 - maxOpponentsStrengths;

#ifdef LOG_POKER_EXEC
    cout << endl
         << "\tsimulation with " << cards << " : " << endl
         << "\t\twin at showdown is " << r.winSd << endl
         << "\t\ttie at showdown is " << r.tieSd << endl
         << "\t\twin now (against random hands)  is " << r.win << endl
         << "\t\twin now (against ranged hands) is " << r.winRanged << endl;
    if (r.winRanged == 0)
        cout << endl
             << "\t\tr.winRanged = 1 - " << maxOpponentsStrengths << " = 0 : setting value to r.win / 4 = " << r.win / 4
             << endl;

    if (getPotOdd() > 0)
        cout << "\t\tpot odd is " << getPotOdd() << endl;

#endif

    if (r.winRanged == 0)
        r.winRanged = r.win / 4;

    return r;
}

float Player::getMaxOpponentsStrengths() const
{

    std::map<int, float> strenghts = evaluateOpponentsStrengths(); // id player --> % of possible hands that beat us

    float maxOpponentsStrengths = 0;
    int opponentID = -1;

    for (std::map<int, float>::const_iterator i = strenghts.begin(); i != strenghts.end(); i++)
    {

        assert(i->second <= 1.0);
        if (i->second > maxOpponentsStrengths)
            maxOpponentsStrengths = i->second;

        opponentID = i->first;
    }

    //	// if we are facing a single opponent in very loose mode, don't adjust the strength
    //	if (strenghts.size() == 1){
    //		std::shared_ptr<Player> opponent = getPlayerByUniqueId(opponentID);
    //
    //		if (opponent->isInVeryLooseMode())
    //			return maxOpponentsStrengths;
    //	}
    //
    //	// adjust roughly maxOpponentsStrengths value according to the number of opponents : the more opponents, the
    // more chances we are beaten
    //
    //	const float originMaxOpponentsStrengths = maxOpponentsStrengths;
    //
    //	if (strenghts.size() == 2) // 2 opponents
    //		maxOpponentsStrengths = maxOpponentsStrengths * 1.1;
    //	else
    //	if (strenghts.size() == 3) // 3 opponents
    //		maxOpponentsStrengths = maxOpponentsStrengths * 1.2;
    //	else
    //	if (strenghts.size() == 4) // 4 opponents
    //		maxOpponentsStrengths = maxOpponentsStrengths * 1.3;
    //	else
    //	if (strenghts.size() > 4)
    //		maxOpponentsStrengths = maxOpponentsStrengths * 1.4;
    //
    //	// adjust roughly maxOpponentsStrengths value according to the number of raises : the more raises, the more
    // chances we are beaten
    //
    //	const int nbRaises =	currentHand->getFlopBetsOrRaisesNumber() +
    //							currentHand->getTurnBetsOrRaisesNumber() +
    //							currentHand->getRiverBetsOrRaisesNumber();
    //
    //	if (nbRaises == 2)
    //		maxOpponentsStrengths = maxOpponentsStrengths * 1.2;
    //	else
    //	if (nbRaises == 3)
    //		maxOpponentsStrengths = maxOpponentsStrengths * 1.5;
    //	else
    //	if (nbRaises == 4)
    //		maxOpponentsStrengths = maxOpponentsStrengths * 2;
    //	else
    //	if (nbRaises > 4)
    //		maxOpponentsStrengths = maxOpponentsStrengths * 3;
    //
    //	if (maxOpponentsStrengths > 1){
    // #ifdef LOG_POKER_EXEC
    //		cout << "\t\tmaxOpponentsStrengths is > 1 : origin value was " << originMaxOpponentsStrengths
    //			<< ", nb raises is " << nbRaises
    //			<< ", nb opponents is " << strenghts.size()
    //			<< " --> setting value to 1" << endl;
    // #endif
    //		maxOpponentsStrengths = 1;
    //	}

    return maxOpponentsStrengths;
}

// get an opponent winning hands % against me, giving his supposed range
float Player::getOpponentWinningHandsPercentage(const int opponentId, std::string board) const
{

    float result = 0;

    std::shared_ptr<Player> opponent = getPlayerByUniqueId(opponentId);

    const int myRank = RankHand((myCard1 + myCard2 + board).c_str());

    // compute winning hands % against my rank
    int nbWinningHands = 0;

    if (opponent->getEstimatedRange().size() == 0)
    {
        computeEstimatedPreflopRange(opponentId);
    }

    vector<std::string> ranges = getRangeAtomicValues(opponent->getEstimatedRange());

    vector<std::string> newRanges;

    for (vector<std::string>::const_iterator i = ranges.begin(); i != ranges.end(); i++)
    {

        if ((*i).size() != 4)
        {
            cout << "invalid hand : " << (*i) << endl;
            continue;
        }
        string s1 = (*i).substr(0, 2);
        string s2 = (*i).substr(2, 4);

        // delete hands that can't exist, given the board (we prefer to filter it here, instead of removing them from
        // the estimated range, for a better GUI readablity (avoid to list numerous particular hands, via the GUI)
        if (board.find(s1) != string::npos || board.find(s2) != string::npos)
            continue;

        // delete hands that can't exist, given the player's cards (if they are supposed to be known)
        if (opponentId != myID)
        {
            if (myCard1.find(s1) != string::npos || myCard2.find(s1) != string::npos ||
                myCard1.find(s2) != string::npos || myCard2.find(s2) != string::npos)
            {
                continue;
            }
        }

        newRanges.push_back(*i);
    }
    if (newRanges.size() == 0)
    {
        newRanges.push_back(ANY_CARDS_RANGE);
    }

    for (vector<std::string>::const_iterator i = newRanges.begin(); i != newRanges.end(); i++)
    {
        // cout << (*i) << endl;
        if (RankHand(((*i) + board).c_str()) > myRank)
            nbWinningHands++;
    }
    if (ranges.size() == 0)
    {
        cout << "Error : no ranges for opponent " << opponentId << endl;
        return 0;
    }
    assert(nbWinningHands / ranges.size() <= 1.0);
    return (float) nbWinningHands / (float) newRanges.size();
}

// purpose : remove some unplausible hands (to my opponents eyes), given what I did preflop
void Player::updateUnplausibleRangesGivenPreflopActions()
{

    computeEstimatedPreflopRange(myID);
    const string originalEstimatedRange = myEstimatedRange;

#ifdef LOG_POKER_EXEC
    std::cout << endl << "\tPlausible range on preflop for player " << myID << " :\t" << myEstimatedRange << endl;
#endif

    const int nbPlayers = currentHand->getActivePlayerList()->size();

    PreflopStatistics stats = getStatistics(nbPlayers).getPreflopStatistics();

    // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more accurate
    if (stats.m_hands < MIN_HANDS_STATISTICS_ACCURATE && nbPlayers < 10 &&
        getStatistics(nbPlayers + 1).getPreflopStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        stats = getStatistics(nbPlayers + 1).getPreflopStatistics();

    // if no raise and the BB checks :
    if (getCurrentHandActions().getPreflopActions().back() == PLAYER_ACTION_CHECK)
    {

        if (stats.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
            myEstimatedRange = substractRange(myEstimatedRange, getStringRange(nbPlayers, stats.getPreflopRaise()));
        else
            myEstimatedRange =
                substractRange(myEstimatedRange, getStringRange(nbPlayers, getStandardRaisingRange(nbPlayers)));
    }

    if (myEstimatedRange == "")
        myEstimatedRange = originalEstimatedRange;

#ifdef LOG_POKER_EXEC
    displayPlausibleRange(GAME_STATE_PREFLOP);
#endif
}

// purpose : remove some unplausible hands, would would normally be in the estimated preflop range, given what I did on
// flop
void Player::updateUnplausibleRangesGivenFlopActions()
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);

    const string originalEstimatedRange = myEstimatedRange;
    string unplausibleRanges;

#ifdef LOG_POKER_EXEC
    std::cout << endl << "\tPlausible range on flop, before update :\t" << myEstimatedRange << endl;
#endif

    // update my unplausible hands (unplausible to my opponents eyes), given what I did on flop

    FlopStatistics flop = stats.getFlopStatistics();

    // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more accurate
    if (flop.m_hands < MIN_HANDS_STATISTICS_ACCURATE / 2 && nbPlayers < 10 &&
        getStatistics(nbPlayers + 1).getPreflopStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        flop = getStatistics(nbPlayers + 1).getFlopStatistics();

    PreflopStatistics preflop = stats.getPreflopStatistics();

    // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more accurate
    if (preflop.m_hands < MIN_HANDS_STATISTICS_ACCURATE / 2 && nbPlayers < 10 &&
        getStatistics(nbPlayers + 1).getPreflopStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        preflop = getStatistics(nbPlayers + 1).getPreflopStatistics();

    if (isInVeryLooseMode())
    {
#ifdef LOG_POKER_EXEC
        std::cout << endl
                  << "\tSeems to be (temporarily ?) on very loose mode : estimated range is\t" << myEstimatedRange
                  << endl;
#endif
        return;
    }

    std::string stringBoard;
    int board[5];
    currentHand->getBoard()->getCards(board);

    for (int i = 0; i < 3; i++)
    {
        stringBoard += " ";
        string card = CardsValue::CardStringValue[board[i]];
        stringBoard += card;
    }

    int nbRaises = 0;
    int nbBets = 0;
    int nbChecks = 0;
    int nbCalls = 0;

    for (std::vector<PlayerAction>::const_iterator i = myCurrentHandActions.m_flopActions.begin();
         i != myCurrentHandActions.m_flopActions.end(); i++)
    {

        if (*i == PLAYER_ACTION_RAISE || *i == PLAYER_ACTION_ALLIN)
            nbRaises++;
        else if (*i == PLAYER_ACTION_BET)
            nbBets++;
        else if (*i == PLAYER_ACTION_CHECK)
            nbChecks++;
        else if (*i == PLAYER_ACTION_CALL)
            nbCalls++;
    }

    vector<std::string> ranges = getRangeAtomicValues(myEstimatedRange);

    for (vector<std::string>::const_iterator i = ranges.begin(); i != ranges.end(); i++)
    {

        string s1 = (*i).substr(0, 2);
        string s2 = (*i).substr(2, 4);

        std::string stringHand = s1 + " " + s2;
        PostFlopState r;
        GetHandState((stringHand + stringBoard).c_str(), &r);

        bool removeHand = false;

        if (myAction == PLAYER_ACTION_CALL)
            removeHand = isUnplausibleHandGivenFlopCall(r, nbRaises, nbBets, nbChecks, nbCalls, flop);
        else if (myAction == PLAYER_ACTION_CHECK)
            removeHand = isUnplausibleHandGivenFlopCheck(r, flop);
        else if (myAction == PLAYER_ACTION_RAISE)
            removeHand = isUnplausibleHandGivenFlopRaise(r, nbRaises, nbBets, nbChecks, nbCalls, flop);
        else if (myAction == PLAYER_ACTION_BET)
            removeHand = isUnplausibleHandGivenFlopBet(r, nbChecks, flop);
        else if (myAction == PLAYER_ACTION_ALLIN)
            removeHand = isUnplausibleHandGivenFlopAllin(r, nbRaises, nbBets, nbChecks, nbCalls, flop);

        if (removeHand)
        {

            string range = s1 + s2;
            string newUnplausibleRange = ",";
            newUnplausibleRange += range;
            newUnplausibleRange += ",";

            if (unplausibleRanges.find(newUnplausibleRange) == string::npos)
                unplausibleRanges += newUnplausibleRange;
        }
    }

    myEstimatedRange = substractRange(myEstimatedRange, unplausibleRanges, stringBoard);

    if (myEstimatedRange == "")
    {
        // keep previous range
#ifdef LOG_POKER_EXEC
        cout << "\tCan't remove all plausible ranges, keeping last one" << endl;
#endif
        myEstimatedRange = originalEstimatedRange;
        unplausibleRanges = "";
    }

#ifdef LOG_POKER_EXEC
    if (unplausibleRanges != "")
        cout << "\tRemoving unplausible ranges : " << unplausibleRanges << endl;
    displayPlausibleRange(GAME_STATE_FLOP);
#endif
}

bool Player::isUnplausibleHandGivenFlopCheck(const PostFlopState& r, const FlopStatistics& flop)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    // the player is in position, he didn't bet on flop, he is not usually passive, and everybody checked on flop :

    if (bHavePosition &&
        !(flop.getAgressionFactor() < 2 && flop.getAgressionFrequency() < 30 &&
          flop.m_hands > MIN_HANDS_STATISTICS_ACCURATE) &&
        (r.UsesFirst || r.UsesSecond))
    {

        // woudn't slow play a medium hand on a dangerous board
        if (!r.IsFullHousePossible &&
            ((r.IsMiddlePair && !r.IsFullHousePossible && currentHand->getRunningPlayerList()->size() < 4) ||
             r.IsTopPair || r.IsOverPair || (r.IsTwoPair && !r.IsFullHousePossible)) &&
            r.IsFlushDrawPossible && r.IsStraightDrawPossible)
            return true;

        // on a non-paired board, he would'nt slow play a straigth, a set or 2 pairs, if a flush draw is possible
        if (!r.IsFullHousePossible && (r.IsTrips || r.IsStraight || r.IsTwoPair) && r.IsFlushDrawPossible)
            return true;

        // wouldn't be passive with a decent hand, on position, if more than 1 opponent
        if (!r.IsFullHousePossible && (r.IsTopPair || r.IsOverPair || r.IsTwoPair || r.IsTrips) &&
            currentHand->getRunningPlayerList()->size() > 2)
            return true;

        // on a paired board, he wouldn't check if he has a pocket overpair
        if (r.IsFullHousePossible && r.IsOverPair)
            return true;
    }
    return false;
}

bool Player::isUnplausibleHandGivenFlopBet(const PostFlopState& r, int nbChecks, const FlopStatistics& flop)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (flop.getAgressionFactor() > 3 && flop.getAgressionFrequency() > 50 &&
        flop.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is very agressive, so don't make any guess

    if (isInVeryLooseMode())
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    // the player made a donk bet on the flop, and is not a maniac player : he should have at least a middle or top pair
    // or a draw
    if (!bHavePosition && !isAgressor(GAME_STATE_PREFLOP))
    {

        if (r.IsOverCards || r.StraightOuts >= 8 || r.FlushOuts >= 8)
            return (currentHand->getRunningPlayerList()->size() > 2 ? true : false);

        if (!((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
              r.IsQuads || r.IsStFlush))
        {

            if (r.IsNoPair)
                return true;

            if (r.IsOnePair)
            {

                if (r.IsFullHousePossible)
                    return true;

                if (!r.IsMiddlePair && !r.IsTopPair && !r.IsOverPair)
                    return true;
            }
        }
    }

    // on a 3 or more players pot : if the player bets in position, he should have at least a middle pair
    if (bHavePosition && currentHand->getRunningPlayerList()->size() > 2)
    {

        if (r.IsOverCards || r.StraightOuts >= 8 || r.FlushOuts >= 8)
            return true;

        if (!((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
              r.IsQuads || r.IsStFlush))
        {

            if (r.IsNoPair)
                return true;

            if (r.IsOnePair)
            {

                if (r.IsFullHousePossible)
                    return true;

                if (!r.IsMiddlePair && !r.IsTopPair && !r.IsOverPair)
                    return true;
            }
        }
    }

    // on a 3 or more players pot : if the player is first to act, and bets, he should have at least a top pair
    if (nbChecks == 0 && currentHand->getRunningPlayerList()->size() > 2)
    {

        if (!((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
              r.IsQuads || r.IsStFlush))
        {

            if (r.IsNoPair)
                return true;

            if (r.IsOnePair)
            {

                if (r.IsFullHousePossible)
                    return true;

                if (!r.IsTopPair && !r.IsOverPair)
                    return true;
            }
        }
    }
    return false;
}

bool Player::isUnplausibleHandGivenFlopCall(const PostFlopState& r, int nbRaises, int nbBets, int nbChecks, int nbCalls,
                                            const FlopStatistics& flop)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (getPotOdd() < 20)
        return false;

    if (isInVeryLooseMode())
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    if (currentHand->getFlopBetsOrRaisesNumber() > 0 && myAction == PLAYER_ACTION_CALL &&
        !(stats.getWentToShowDown() > 35 && stats.getRiverStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE))
    {

        if (!((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
              r.IsQuads || r.IsStFlush || r.IsOverCards || r.FlushOuts >= 8 || r.StraightOuts >= 8))
        {

            if (r.IsNoPair)
                return true;

            if (currentHand->getFlopBetsOrRaisesNumber() > 1 && r.IsOnePair && !r.IsTopPair && !r.IsOverPair)
                return true;

            if (currentHand->getFlopBetsOrRaisesNumber() > 2 && (r.IsOnePair || r.IsOverCards))
                return true;

            if (currentHand->getRunningPlayerList()->size() > 2 && r.IsOnePair && !r.IsTopPair && !r.IsOverPair)
                return true;
        }
    }
    return false;
}

bool Player::isUnplausibleHandGivenFlopRaise(const PostFlopState& r, int nbRaises, int nbBets, int nbChecks,
                                             int nbCalls, const FlopStatistics& flop)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (flop.getAgressionFactor() > 3 && flop.getAgressionFrequency() > 50 &&
        flop.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (isInVeryLooseMode())
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    // the player has check-raised the flop, and is not a maniac player : he should have at least a top pair or a draw
    if (nbChecks == 1)
    {

        if ((r.IsOverCards || r.FlushOuts >= 8 || r.StraightOuts >= 8) &&
            currentHand->getRunningPlayerList()->size() > 2)
            return true;

        if (!((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
              r.IsQuads || r.IsStFlush))
        {

            if (r.IsNoPair)
                return true;

            if (r.IsOnePair && !r.IsFullHousePossible && !r.IsTopPair && !r.IsOverPair)
                return true;
        }
    }

    // the player has raised or reraised the flop, and is not a maniac player : he should have at least a top pair
    if (nbRaises > 0)
    {

        if (!((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
              r.IsQuads || r.IsStFlush))
        {

            if (r.IsNoPair || (r.IsOnePair && r.IsFullHousePossible))
                return true;

            if (r.IsOnePair && !r.IsFullHousePossible && !r.IsTopPair & !r.IsOverPair)
                return true;

            if (currentHand->getFlopBetsOrRaisesNumber() > 3 && (r.IsOnePair))
                return true;

            if (currentHand->getFlopBetsOrRaisesNumber() > 4 && (r.IsTwoPair))
                return true;
        }
    }

    return false;
}

bool Player::isUnplausibleHandGivenFlopAllin(const PostFlopState& r, int nbRaises, int nbBets, int nbChecks,
                                             int nbCalls, const FlopStatistics& flop)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (getPotOdd() < 20)
        return false;

    if (flop.getAgressionFactor() > 3 && flop.getAgressionFrequency() > 50 &&
        flop.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (isInVeryLooseMode())
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    if (!((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
          r.IsQuads || r.IsStFlush))
    {

        if (r.IsNoPair || (r.IsOnePair && r.IsFullHousePossible))
            return true;

        if (r.IsOnePair && !r.IsFullHousePossible && !r.IsTopPair & !r.IsOverPair)
            return true;

        if (currentHand->getFlopBetsOrRaisesNumber() > 3 && (r.IsOnePair))
            return true;

        if (currentHand->getFlopBetsOrRaisesNumber() > 4 && (r.IsTwoPair))
            return true;
    }
    return false;
}
// purpose : remove some unplausible hands, who would normally be in the estimated preflop range
void Player::updateUnplausibleRangesGivenTurnActions()
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    const PlayerStatistics& stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());
    const string originalEstimatedRange = myEstimatedRange;
    string unplausibleRanges;

#ifdef LOG_POKER_EXEC
    std::cout << endl << "\tPlausible range on turn, before update :\t" << myEstimatedRange << endl;
#endif

    // update my unplausible hands (unplausible to my opponents eyes), given what I did on turn

    TurnStatistics turn = stats.getTurnStatistics();

    // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more accurate
    if (turn.m_hands < MIN_HANDS_STATISTICS_ACCURATE / 3 && nbPlayers < 10 &&
        getStatistics(nbPlayers + 1).getPreflopStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        turn = getStatistics(nbPlayers + 1).getTurnStatistics();

    PreflopStatistics preflop = stats.getPreflopStatistics();

    // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more accurate
    if (preflop.m_hands < MIN_HANDS_STATISTICS_ACCURATE / 2 && nbPlayers < 10 &&
        getStatistics(nbPlayers + 1).getPreflopStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        preflop = getStatistics(nbPlayers + 1).getPreflopStatistics();

    if (isInVeryLooseMode())
    {
#ifdef LOG_POKER_EXEC
        std::cout << endl
                  << "\tSeems to be (temporarily ?) on very loose mode : estimated range is\t" << myEstimatedRange
                  << endl;
#endif
        return;
    }

    std::string stringBoard;
    int board[5];
    currentHand->getBoard()->getCards(board);

    for (int i = 0; i < 4; i++)
    {
        stringBoard += " ";
        string card = CardsValue::CardStringValue[board[i]];
        stringBoard += card;
    }

    int nbRaises = 0;
    int nbBets = 0;
    int nbChecks = 0;
    int nbCalls = 0;

    for (std::vector<PlayerAction>::const_iterator i = myCurrentHandActions.m_turnActions.begin();
         i != myCurrentHandActions.m_turnActions.end(); i++)
    {

        if (*i == PLAYER_ACTION_RAISE || *i == PLAYER_ACTION_ALLIN)
            nbRaises++;
        else if (*i == PLAYER_ACTION_BET)
            nbBets++;
        else if (*i == PLAYER_ACTION_CHECK)
            nbChecks++;
        else if (*i == PLAYER_ACTION_CALL)
            nbCalls++;
    }

    vector<std::string> ranges = getRangeAtomicValues(myEstimatedRange);

    for (vector<std::string>::const_iterator i = ranges.begin(); i != ranges.end(); i++)
    {

        string s1 = (*i).substr(0, 2);
        string s2 = (*i).substr(2, 4);

        std::string stringHand = s1 + " " + s2;
        PostFlopState r;
        GetHandState((stringHand + stringBoard).c_str(), &r);

        bool removeHand = false;

        if (myAction == PLAYER_ACTION_CALL)
            removeHand = isUnplausibleHandGivenTurnCall(r, nbRaises, nbBets, nbChecks, nbCalls, turn);
        else if (myAction == PLAYER_ACTION_CHECK)
            removeHand = isUnplausibleHandGivenTurnCheck(r, turn);
        else if (myAction == PLAYER_ACTION_RAISE)
            removeHand = isUnplausibleHandGivenTurnRaise(r, nbRaises, nbBets, nbChecks, nbCalls, turn);
        else if (myAction == PLAYER_ACTION_BET)
            removeHand = isUnplausibleHandGivenTurnBet(r, nbChecks, turn);
        else if (myAction == PLAYER_ACTION_ALLIN)
            removeHand = isUnplausibleHandGivenTurnAllin(r, nbRaises, nbBets, nbChecks, nbCalls, turn);

        if (removeHand)
        {

            string range = s1 + s2;
            string newUnplausibleRange = ",";
            newUnplausibleRange += range;
            newUnplausibleRange += ",";

            if (unplausibleRanges.find(newUnplausibleRange) == string::npos)
                unplausibleRanges += newUnplausibleRange;
        }
    }

    myEstimatedRange = substractRange(myEstimatedRange, unplausibleRanges, stringBoard);

    if (myEstimatedRange == "")
    {
        // keep previous range
#ifdef LOG_POKER_EXEC
        cout << "\tCan't remove all plausible ranges, keeping last one" << endl;
#endif
        myEstimatedRange = originalEstimatedRange;
        unplausibleRanges = "";
    }

#ifdef LOG_POKER_EXEC
    if (unplausibleRanges != "")
        cout << "\tRemoving unplausible ranges : " << unplausibleRanges << endl;
    displayPlausibleRange(GAME_STATE_TURN);
#endif
}

bool Player::isUnplausibleHandGivenTurnCheck(const PostFlopState& r, const TurnStatistics& turn)
{

    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    // the player is in position, he isn't usually passive, and everybody checked
    if (bHavePosition && !(turn.getAgressionFactor() < 2 && turn.getAgressionFrequency() < 30 &&
                           turn.m_hands > MIN_HANDS_STATISTICS_ACCURATE))
    {

        if (r.IsPocketPair && r.IsOverPair)
            return true;

        // woudn't slow play a medium hand on a dangerous board, if there was no action on flop
        if (((r.UsesFirst || r.UsesSecond) && currentHand->getFlopBetsOrRaisesNumber() == 0 && r.IsTopPair ||
             (r.IsTwoPair && !r.IsFullHousePossible) || r.IsTrips) &&
            r.IsFlushDrawPossible)
            return true;

        // wouldn't be passive with a decent hand, on position, if more than 1 opponent
        if (((r.UsesFirst || r.UsesSecond) && ((r.IsTwoPair && !r.IsFullHousePossible) || r.IsTrips)) &&
            currentHand->getRunningPlayerList()->size() > 2)
            return true;
    }

    return false;
}

bool Player::isUnplausibleHandGivenTurnBet(const PostFlopState& r, int nbChecks, const TurnStatistics& turn)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (turn.getAgressionFactor() > 3 && turn.getAgressionFrequency() > 50 &&
        turn.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (isInVeryLooseMode())
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    // the player made a donk bet on turn, and is not a maniac player : he should have at least a top pair
    if (!bHavePosition && !isAgressor(GAME_STATE_FLOP) && currentHand->getFlopBetsOrRaisesNumber() > 0)
    {

        if ((r.IsOverCards || r.FlushOuts >= 8 || r.StraightOuts >= 8) &&
            currentHand->getRunningPlayerList()->size() > 2)
            return true;

        if (!((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
              r.IsQuads || r.IsStFlush))
        {

            if (r.IsNoPair)
                return true;

            if (r.IsOnePair && !r.IsTopPair && !r.IsOverPair && !r.IsFullHousePossible)
                return true;
        }
    }

    return false;
}

bool Player::isUnplausibleHandGivenTurnCall(const PostFlopState& r, int nbRaises, int nbBets, int nbChecks, int nbCalls,
                                            const TurnStatistics& turn)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (getPotOdd() < 20)
        return false;

    if (isInVeryLooseMode())
        return false; // he is (temporarily ?) very loose, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    // the player called a bet on flop and turn, and he is not loose
    if (currentHand->getTurnBetsOrRaisesNumber() > 0 && currentHand->getFlopBetsOrRaisesNumber() > 0 &&
        myAction == PLAYER_ACTION_CALL && !isAgressor(GAME_STATE_FLOP) &&
        !(stats.getWentToShowDown() > 30 && stats.getRiverStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE))
    {

        if (!((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
              r.IsQuads || r.IsStFlush || r.FlushOuts >= 8 || r.StraightOuts >= 8))
        {

            if (r.IsNoPair || (r.IsOnePair && r.IsFullHousePossible))
                return true;

            if (r.IsOnePair && !r.IsTopPair && !r.IsOverPair && !r.IsFullHousePossible)
                return true;

            if (currentHand->getTurnBetsOrRaisesNumber() > 2 && r.IsOnePair)
                return true;
        }
    }
    // the player called a raise on turn, and is not loose : he has at least a top pair or a good draw
    if (currentHand->getTurnBetsOrRaisesNumber() > 1 && myAction == PLAYER_ACTION_CALL &&
        !(stats.getWentToShowDown() > 35 && stats.getRiverStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE))
    {

        if (!(((r.IsTwoPair && !r.IsFullHousePossible) && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush ||
              r.IsFullHouse || r.IsTrips || r.IsQuads || r.IsStFlush || r.IsOverCards || r.IsFlushDrawPossible ||
              r.FlushOuts >= 8 || r.StraightOuts >= 8))
        {

            if (r.IsNoPair || (r.IsOnePair && r.IsFullHousePossible))
                return true;

            if (r.IsOnePair && !r.IsTopPair && !r.IsFullHousePossible && !r.IsOverPair)
                return true;
        }
    }

    return false;
}

bool Player::isUnplausibleHandGivenTurnRaise(const PostFlopState& r, int nbRaises, int nbBets, int nbChecks,
                                             int nbCalls, const TurnStatistics& turn)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (turn.getAgressionFactor() > 3 && turn.getAgressionFrequency() > 50 &&
        turn.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is very agressive, so don't make any guess

    if (isInVeryLooseMode())
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    // if nobody has bet the flop, he should at least have a top pair
    if (currentHand->getFlopBetsOrRaisesNumber() == 0)
    {

        if (r.IsTopPair || r.IsOverPair || (r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush ||
            r.IsFullHouse || r.IsTrips || r.IsQuads || r.IsStFlush)
            return false;
        else
            return true;
    }
    // if he was not the agressor on flop, and an other player has bet the flop, then he should have at least a top pair
    if (!isAgressor(GAME_STATE_FLOP) && currentHand->getFlopBetsOrRaisesNumber() > 0)
    {

        if (r.IsTopPair || r.IsOverPair || (r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush ||
            r.IsFullHouse || r.IsTrips || r.IsQuads || r.IsStFlush)
            return false;
        else
            return true;
    }
    // the player has raised twice the turn, and is not a maniac player : he should have at least two pairs
    if (nbRaises == 2 && !((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse ||
                           r.IsTrips || r.IsQuads || r.IsStFlush))
        return true;

    // the player has raised 3 times the turn, and is not a maniac player : he should have better than a set
    if (nbRaises > 2 && !(r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsQuads || r.IsStFlush))
        return true;

    return false;
}

bool Player::isUnplausibleHandGivenTurnAllin(const PostFlopState& r, int nbRaises, int nbBets, int nbChecks,
                                             int nbCalls, const TurnStatistics& turn)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (getPotOdd() < 20)
        return false;

    if (turn.getAgressionFactor() > 3 && turn.getAgressionFrequency() > 50 &&
        turn.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (isInVeryLooseMode())
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    // the player has raised twice the turn, and is not a maniac player : he should have at least two pairs
    if (nbRaises == 2 && !((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse ||
                           r.IsTrips || r.IsQuads || r.IsStFlush))
        return true;

    // the player has raised 3 times the turn, and is not a maniac player : he should have better than a set
    if (nbRaises > 2 && !(r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsQuads || r.IsStFlush))
        return true;

    return false;
}
// purpose : remove some unplausible hands, woul would normally be in the estimated preflop range
void Player::updateUnplausibleRangesGivenRiverActions()
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    const PlayerStatistics& stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());
    const string originalEstimatedRange = myEstimatedRange;
    string unplausibleRanges;

#ifdef LOG_POKER_EXEC
    std::cout << endl << "\tPlausible range on river, before update :\t" << myEstimatedRange << endl;
#endif

    // update my unplausible hands (unplausible to my opponents eyes), given what I did on river

    RiverStatistics river = stats.getRiverStatistics();

    // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more accurate
    if (river.m_hands < MIN_HANDS_STATISTICS_ACCURATE / 3 && nbPlayers < 10 &&
        getStatistics(nbPlayers + 1).getPreflopStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        river = getStatistics(nbPlayers + 1).getRiverStatistics();

    PreflopStatistics preflop = stats.getPreflopStatistics();

    // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more accurate
    if (preflop.m_hands < MIN_HANDS_STATISTICS_ACCURATE / 2 && nbPlayers < 10 &&
        getStatistics(nbPlayers + 1).getPreflopStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        preflop = getStatistics(nbPlayers + 1).getPreflopStatistics();

    if (isInVeryLooseMode())
    {
#ifdef LOG_POKER_EXEC
        std::cout << endl << "\tSeems to be on very loose mode : estimated range is\t" << myEstimatedRange << endl;
#endif
        return;
    }

    std::string stringBoard;
    int board[5];
    currentHand->getBoard()->getCards(board);

    for (int i = 0; i < 5; i++)
    {
        stringBoard += " ";
        string card = CardsValue::CardStringValue[board[i]];
        stringBoard += card;
    }

    int nbRaises = 0;
    int nbBets = 0;
    int nbChecks = 0;
    int nbCalls = 0;

    for (std::vector<PlayerAction>::const_iterator i = myCurrentHandActions.m_riverActions.begin();
         i != myCurrentHandActions.m_riverActions.end(); i++)
    {

        if (*i == PLAYER_ACTION_RAISE || *i == PLAYER_ACTION_ALLIN)
            nbRaises++;
        else if (*i == PLAYER_ACTION_BET)
            nbBets++;
        else if (*i == PLAYER_ACTION_CHECK)
            nbChecks++;
        else if (*i == PLAYER_ACTION_CALL)
            nbCalls++;
    }

    vector<std::string> ranges = getRangeAtomicValues(myEstimatedRange);

    for (vector<std::string>::const_iterator i = ranges.begin(); i != ranges.end(); i++)
    {

        string s1 = (*i).substr(0, 2);
        string s2 = (*i).substr(2, 4);

        std::string stringHand = s1 + " " + s2;
        PostFlopState r;
        GetHandState((stringHand + stringBoard).c_str(), &r);

        bool removeHand = false;

        if (myAction == PLAYER_ACTION_CALL)
            removeHand = isUnplausibleHandGivenRiverCall(r, nbRaises, nbBets, nbChecks, nbCalls, river);
        else if (myAction == PLAYER_ACTION_CHECK)
            removeHand = isUnplausibleHandGivenRiverCheck(r, river);
        else if (myAction == PLAYER_ACTION_RAISE)
            removeHand = isUnplausibleHandGivenRiverRaise(r, nbRaises, nbBets, nbChecks, nbCalls, river);
        else if (myAction == PLAYER_ACTION_BET)
            removeHand = isUnplausibleHandGivenRiverBet(r, nbChecks, river);
        else if (myAction == PLAYER_ACTION_ALLIN)
            removeHand = isUnplausibleHandGivenRiverAllin(r, nbRaises, nbBets, nbChecks, nbCalls, river);

        if (removeHand)
        {

            string range = s1 + s2;
            string newUnplausibleRange = ",";
            newUnplausibleRange += range;
            newUnplausibleRange += ",";

            if (unplausibleRanges.find(newUnplausibleRange) == string::npos)
                unplausibleRanges += newUnplausibleRange;
        }
    }

    myEstimatedRange = substractRange(myEstimatedRange, unplausibleRanges, stringBoard);

    if (myEstimatedRange == "")
    {
        // keep previous range
#ifdef LOG_POKER_EXEC
        cout << "\tCan't remove all plausible ranges, keeping last one" << endl;
#endif
        myEstimatedRange = originalEstimatedRange;
        unplausibleRanges = "";
    }

#ifdef LOG_POKER_EXEC
    if (unplausibleRanges != "")
        cout << "\tRemoving unplausible ranges : " << unplausibleRanges << endl;
    displayPlausibleRange(GAME_STATE_RIVER);
#endif
}

bool Player::isUnplausibleHandGivenRiverCheck(const PostFlopState& r, const RiverStatistics& river)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    // todo

    return false;
}

bool Player::isUnplausibleHandGivenRiverBet(const PostFlopState& r, int nbChecks, const RiverStatistics& river)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (river.getAgressionFactor() > 3 && river.getAgressionFrequency() > 50 &&
        river.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (isInVeryLooseMode())
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    // the player has bet the river, was not the agressor on turn and river, and is not a maniac player : he should have
    // at least 2 pairs
    if (currentHand->getFlopBetsOrRaisesNumber() > 1 && !isAgressor(GAME_STATE_FLOP) &&
        currentHand->getTurnBetsOrRaisesNumber() > 1 && !isAgressor(GAME_STATE_TURN))
    {

        if ((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
            r.IsQuads || r.IsStFlush)
            return false;
        else
            return true;
    }

    // the player has bet the river, is out of position on a multi-players pot, in a hand with some action, and is not a
    // maniac player : he should have at least 2 pairs
    if (!bHavePosition && currentHand->getRunningPlayerList()->size() > 2 &&
        ((currentHand->getFlopBetsOrRaisesNumber() > 1 && !isAgressor(GAME_STATE_FLOP)) ||
         (currentHand->getTurnBetsOrRaisesNumber() > 1 && !isAgressor(GAME_STATE_TURN))))
    {

        if ((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
            r.IsQuads || r.IsStFlush)
            return false;
        else
            return true;
    }
    return false;
}

bool Player::isUnplausibleHandGivenRiverCall(const PostFlopState& r, int nbRaises, int nbBets, int nbChecks,
                                             int nbCalls, const RiverStatistics& river)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (getPotOdd() < 20)
        return false;

    if (river.getAgressionFactor() > 3 && river.getAgressionFrequency() > 50 &&
        river.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (isInVeryLooseMode())
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    // the player has called the river on a multi-players pot, and is not a loose player : he should have at least a top
    // pair
    if (currentHand->getRunningPlayerList()->size() > 2)
    {

        if (!((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
              r.IsQuads || r.IsStFlush))
        {

            if (r.IsNoPair || (r.IsOnePair && r.IsFullHousePossible))
                return true;

            if (r.IsOnePair && !r.IsTopPair & !r.IsOverPair)
                return true;
        }
    }

    return false;
}

bool Player::isUnplausibleHandGivenRiverRaise(const PostFlopState& r, int nbRaises, int nbBets, int nbChecks,
                                              int nbCalls, const RiverStatistics& river)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (river.getAgressionFactor() > 3 && river.getAgressionFrequency() > 50 &&
        river.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (isInVeryLooseMode())
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    // the player has raised the river, and is not a maniac player : he should have at least 2 pairs
    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    if (r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips || r.IsQuads || r.IsStFlush ||
        (r.IsTwoPair && !r.IsFullHousePossible))
        return false;
    else
        return true;

    // the player has raised the river, is out of position on a multi-players pot, in a hand with some action, and is
    // not a maniac player : he should have at least a set
    if (!bHavePosition && currentHand->getRunningPlayerList()->size() > 2 &&
        ((currentHand->getFlopBetsOrRaisesNumber() > 1 && !isAgressor(GAME_STATE_FLOP)) ||
         (currentHand->getTurnBetsOrRaisesNumber() > 1 && !isAgressor(GAME_STATE_TURN))))
    {

        if (r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips || r.IsQuads || r.IsStFlush)
            return false;
        else
            return true;
    }

    // the player has raised twice the river, and is not a maniac player : he should have at least trips
    if (nbRaises == 2 && !(r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips || r.IsQuads || r.IsStFlush))
        return true;

    // the player has raised 3 times the river, and is not a maniac player : he should have better than a set
    if (nbRaises > 2 && !(r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsQuads || r.IsStFlush))
        return true;

    return false;
}

bool Player::isUnplausibleHandGivenRiverAllin(const PostFlopState& r, int nbRaises, int nbBets, int nbChecks,
                                              int nbCalls, const RiverStatistics& river)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (getPotOdd() < 20)
        return false;

    if (river.getAgressionFactor() > 3 && river.getAgressionFrequency() > 50 &&
        river.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (isInVeryLooseMode())
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    // the player has raised twice or more the river, and is not a maniac player : he should have at least a straight
    if (nbRaises > 1 && !(r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsQuads || r.IsStFlush))
        return true;

    return false;
}

std::string Player::substractRange(const std::string originRanges, const std::string rangesToSubstract,
                                   const std::string board)
{

    std::istringstream oss(originRanges);
    std::string singleOriginRange;
    string newRange;

    while (getline(oss, singleOriginRange, ','))
    {

        if (singleOriginRange.size() == 0)
            continue;

        // singleOriginRange may contain 1 range of any type, i.e : "AJo+"  "99"  "22+"  "87s" , or even a real hand
        // like "As3c"

        vector<std::string> cardsInOriginRange = getRangeAtomicValues(
            singleOriginRange); // split this range (if needed) into real cards, like ",AhJc,AsQc,....."

        bool keepOriginRange = true;

        for (vector<std::string>::const_iterator originHand = cardsInOriginRange.begin();
             originHand != cardsInOriginRange.end(); originHand++)
        {

            const string originCard1 = (*originHand).substr(0, 2);
            const string originCard2 = (*originHand).substr(2, 4);

            if (isCardsInRange(originCard1, originCard2, rangesToSubstract))
            {

                keepOriginRange = false; // at least one hand must be substracted from the singleOriginRange, so we must
                                         // replace this range by an other (smaller) range

                // check if this hand has been previously included in the new range, when processing an other range
                std::string::size_type pos = newRange.find(*originHand);
                if (pos != std::string::npos)
                {
#ifdef LOG_POKER_EXEC
                    std::cout << "removing previously included hand";
#endif
                    newRange = newRange.erase(pos, 4);
#ifdef LOG_POKER_EXEC
                    std::cout << "...new range is now " << newRange << endl;
#endif
                    continue;
                }

                // cout << endl << "must remove " << originCard1 << originCard2 << endl;

                vector<std::string> atomicRangesInSingleOriginRange = getRangeAtomicValues(singleOriginRange, true);
                // atomicRangesInSingleOriginRange will now contain the singleOriginRange ranges, but without + or -
                // signs. It may also contain real hands, like 5h4h. purpose : we will try to keep as few "real hands"
                // as possible, for better display readability via the GUI

                for (vector<std::string>::const_iterator atomicOriginRange = atomicRangesInSingleOriginRange.begin();
                     atomicOriginRange != atomicRangesInSingleOriginRange.end(); atomicOriginRange++)
                {

                    // std::cout << "single origin atomic range is " << *atomicOriginRange << endl;

                    // if the "range" is in fact a real hand :
                    if (atomicOriginRange->size() == 4)
                    {

                        const string s1 = (*atomicOriginRange).substr(0, 2);
                        const string s2 = (*atomicOriginRange).substr(2, 4);
                        if (isCardsInRange(s1, s2, rangesToSubstract))
                            continue;
                        // delete hands that can't exist, given the board
                        if (board.find(s1) != string::npos || board.find(s2) != string::npos)
                            continue;
                        if (newRange.find(*atomicOriginRange) == string::npos)
                            newRange += "," + (*atomicOriginRange); // don't put it twice in the new range

                        continue;
                    }

                    // process the real ranges like AQo AKo A5s 55 77 ....(i.e, atomic ranges, with no + or -)

                    if (originCard1.at(1) == originCard2.at(1))
                    {

                        // if we are processing a suited hand

                        string suitedRanges;
                        int nbSuitedRanges = 0;

                        vector<std::string> handsInAtomicRange = getRangeAtomicValues(*atomicOriginRange);

                        for (vector<std::string>::const_iterator i = handsInAtomicRange.begin();
                             i != handsInAtomicRange.end(); i++)
                        {

                            // std::cout << "hand in atomic range is " << *i << endl;

                            if (newRange.find(*i) != string::npos)
                                continue; // don't put it twice in the new range

                            const string s1 = (*i).substr(0, 2);
                            const string s2 = (*i).substr(2, 4);

                            if (isCardsInRange(s1, s2, rangesToSubstract))
                                continue;

                            // delete hands that can't exist, given the board
                            if (board.find(s1) != string::npos || board.find(s2) != string::npos)
                                continue;

                            // std::cout << "we keep it" << endl;
                            nbSuitedRanges++;
                            suitedRanges += "," + (*i); // we keep this hand
                        }

                        if (nbSuitedRanges < 4)
                        {
                            newRange += suitedRanges; // put the real hands, like AhJh AsJs
                        }
                        else
                        {
                            // put a range like AJs, for better readability, instead of putting "AhJh AdJd AcJc AsJs"
                            if (newRange.find(*atomicOriginRange) == string::npos)
                                newRange += "," + (*atomicOriginRange); // don't put it twice in the new range
                        }
                        // cout << "new range is now " << newRange << endl;
                    }

                    else
                    {

                        // unsuited hands, including pairs

                        vector<std::string> handsInAtomicRange = getRangeAtomicValues(*atomicOriginRange);

                        for (vector<std::string>::const_iterator i = handsInAtomicRange.begin();
                             i != handsInAtomicRange.end(); i++)
                        {

                            // std::cout << "hand in atomic range is " << *i << endl;

                            const string s1 = (*i).substr(0, 2);
                            const string s2 = (*i).substr(2, 4);

                            if (isCardsInRange(s1, s2, rangesToSubstract))
                                continue;

                            if (newRange.find(*atomicOriginRange) != string::npos)
                                continue; // don't put it twice in the new range

                            // delete hands that can't exist, given the board
                            if (board.find(s1) != string::npos || board.find(s2) != string::npos)
                                continue;

                            // std::cout << "we keep it" << endl;
                            newRange += "," + (*atomicOriginRange); // we keep this range
                        }
                    }
                }
            }
        }

        if (keepOriginRange)
        { // all hands in the origin range are kept
            newRange += ",";
            newRange += singleOriginRange;
        }
    }
    // unsigned pos;
    // while ((pos = newRange.find(",,")) != string::npos)
    // newRange = newRange.replace(pos, 2, ",");

    return newRange;
}

void Player::displayPlausibleRange(GameState g)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    string label;
    char bettingRound;

    if (g == GAME_STATE_PREFLOP)
    {
        label = "preflop";
        bettingRound = 'P';
    }
    else if (g == GAME_STATE_FLOP)
    {
        label = "flop";
        bettingRound = 'F';
    }
    else if (g == GAME_STATE_TURN)
    {
        label = "turn";
        bettingRound = 'T';
    }
    else if (g == GAME_STATE_RIVER)
    {
        label = "river";
        bettingRound = 'R';
    }
    // if some hands are not plausible any more within the estimated starting range, substract them from the starting
    // range,

    if (!isCardsInRange(myCard1, myCard2, myEstimatedRange))
    {
        std::cout << endl
                  << "\t" << myCard1 << " " << myCard2 << " isn't part of the plausible " << label << " range :\t"
                  << myEstimatedRange << endl;
        currentHand->getHandAuditStore()->updateUnplausibleHand(myCard1, myCard2, (myID == 0 ? true : false),
                                                                bettingRound, nbPlayers);
    }
    else
        std::cout << endl
                  << "\t" << myCard1 << " " << myCard2 << " is part of the plausible " << label << " range :\t"
                  << myEstimatedRange << endl;
}

std::string Player::getHandToRange(const std::string card1, const std::string card2) const
{

    // receive a hand like Ac Ks and translate it into a range like AKo

    std::stringstream result;

    if (card1.size() != 2 || card2.size() != 2)
    {
        cout << "Player::getHandToRange invalid hand : " << card1 << " " << card2 << endl;
        return "";
    }

    if (card1.at(0) == card2.at(0))
        result << card2.at(0) << card2.at(0); // pair
    else if (card1.at(1) == card2.at(1))      // suited hand
        result << card1.at(0) << card2.at(0) << "s";
    else
        result << card1.at(0) << card2.at(0) << "o";

    return result.str();
}

void Player::DisplayHandState(const PostFlopState* state) const
{

#ifdef LOG_POKER_EXEC

    cout << endl << "\t";

    if (!state->UsesFirst && !state->UsesSecond)
        cout << "Playing the board, ";
    else if (state->UsesFirst && !state->UsesSecond)
        cout << "Using only first hole card, ";
    else if (!state->UsesFirst && state->UsesSecond)
        cout << "Using only second hole card, ";
    else if (state->UsesFirst && state->UsesSecond)
        cout << "Using both hole cards, ";

    if (state->IsNoPair)
    {
        if (state->IsOverCards)
            cout << "with over cards";
        else
            cout << "with high card";
    }
    else if (state->IsOnePair)
    {
        if (state->IsTopPair)
            cout << "with top pair";
        else if (state->IsMiddlePair)
            cout << "with middle pair";
        else if (state->IsBottomPair)
            cout << "with bottom pair";
        else if (state->IsOverPair)
            cout << "with over pair";
        else if (state->IsPocketPair)
            cout << "with pocket pair";
        else if (state->IsFullHousePossible)
            cout << "with one pair on board";
        else
            cout << "with one pair ";
    }
    else if (state->IsTwoPair)
        cout << "with two pair";
    else if (state->IsTrips)
        cout << "with trips";
    else if (state->IsStraight)
        cout << "with a straight";
    else if (state->IsFlush)
        cout << "with a flush";
    else if (state->IsFullHouse)
        cout << "with a full house";
    else if (state->IsQuads)
        cout << "with quads";
    else if (state->IsStFlush)
        cout << "with a straight flush";

    // Do we have a flush and straight draw?
    bool flushDraw = (state->Is3Flush || state->Is4Flush);
    bool straightDraw = (state->StraightOuts);
    bool anotherDraw = (flushDraw && straightDraw);

    if (anotherDraw)
        cout << ", ";
    else if (flushDraw || straightDraw)
        cout << " and ";

    if (state->Is4Flush)
        cout << "4 flush cards (" << state->FlushOuts << " outs)";
    else if (state->Is3Flush)
        cout << "3 flush cards";

    if (anotherDraw)
        cout << ", and ";

    if (straightDraw)
        cout << state->StraightOuts << " outs to a straight";

    cout << ".";

    if (state->BetterOuts)
        cout << " " << state->BetterOuts << " outs to boat or better.";

    if (state->IsFlushPossible)
        cout << " Someone may have a flush.";
    else if (state->IsFlushDrawPossible)
        cout << " Someone may be drawing to a flush.";

    if (state->IsStraightPossible)
        cout << " Someone may have a straight.";
    else if (state->IsStraightDrawPossible)
        cout << " Someone may be drawing to a straight.";

    if (state->IsFullHousePossible)
        cout << " The board is paired.";

#endif
}

std::map<int, float> Player::evaluateOpponentsStrengths() const
{

    map<int, float> result;
    PlayerList players = currentHand->getActivePlayerList();
    const int nbPlayers = currentHand->getActivePlayerList()->size();

#ifdef LOG_POKER_EXEC
    cout << endl;
#endif

    for (PlayerListIterator it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getID() == myID || (*it)->getAction() == PLAYER_ACTION_FOLD)
            continue;

        const float estimatedOpponentWinningHands = getOpponentWinningHandsPercentage((*it)->getID(), getStringBoard());
        assert(estimatedOpponentWinningHands <= 1.0);

        result[(*it)->getID()] = estimatedOpponentWinningHands;
    }

    return result;
}

void Player::computeEstimatedPreflopRange(const int opponentId) const
{

    using std::cout;
    const int nbPlayers = currentHand->getActivePlayerList()->size();
    std::shared_ptr<Player> opponent = getPlayerByUniqueId(opponentId);
    const int lastRaiserID = currentHand->getPreflopLastRaiserID();

#ifdef LOG_POKER_EXEC
    std::cout << endl
              << "\t\testimated range for " << (opponentId == 0 ? "Human player" : opponent->getName()) << " : ";
#endif

    PreflopStatistics stats = opponent->getStatistics(nbPlayers).getPreflopStatistics();

    // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more accurate
    if (stats.m_hands < MIN_HANDS_STATISTICS_ACCURATE && nbPlayers < 10 &&
        opponent->getStatistics(nbPlayers + 1).getPreflopStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        stats = opponent->getStatistics(nbPlayers + 1).getPreflopStatistics();

#ifdef LOG_POKER_EXEC
    std::cout << "  " << stats.getVoluntaryPutMoneyInPot() << "/" << stats.getPreflopRaise()
              << ", 3B: " << stats.getPreflop3Bet() << ", 4B: " << stats.getPreflop4Bet()
              << ", C3B: " << stats.getPreflopCall3BetsFrequency() << ", pot odd: " << opponent->getPreflopPotOdd()
              << " " << endl
              << "\t\t";
#endif

    // if the player was BB and has checked preflop, then he can have anything, except his approximative BB raising
    // range
    if (currentHand->getPreflopRaisesNumber() == 0 && opponent->getPosition() == BB)
    {

#ifdef LOG_POKER_EXEC
        cout << "any cards except ";
        if (stats.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
            cout << getStringRange(nbPlayers, stats.getPreflopRaise() * 0.8);
        else
            cout << getStringRange(nbPlayers, getStandardRaisingRange(nbPlayers) * 0.8);
        cout << endl;
#endif

        if (stats.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
            opponent->setEstimatedRange(
                opponent->substractRange(ANY_CARDS_RANGE, getStringRange(nbPlayers, stats.getPreflopRaise() * 0.8)));
        else
            opponent->setEstimatedRange(opponent->substractRange(
                ANY_CARDS_RANGE, getStringRange(nbPlayers, getStandardRaisingRange(nbPlayers) * 0.8)));

        return;
    }

    string estimatedRange;

    // if the player is the last raiser :
    if (opponent->getID() == lastRaiserID)
        estimatedRange = computeEstimatedPreflopRangeFromLastRaiser(opponentId, stats);
    else
        estimatedRange = computeEstimatedPreflopRangeFromCaller(opponentId, stats);

#ifdef LOG_POKER_EXEC
    cout << " {" << estimatedRange << "}" << endl;
#endif

    opponent->setEstimatedRange(estimatedRange);
}

string Player::computeEstimatedPreflopRangeFromLastRaiser(const int opponentId, PreflopStatistics& opponentStats) const
{

    using std::cout;
    const int nbPlayers = currentHand->getActivePlayerList()->size();
    std::shared_ptr<Player> opponent = getPlayerByUniqueId(opponentId);

    float range = 0;

#ifdef LOG_POKER_EXEC
    cout << " [ player is last raiser ] " << endl << "\t\t";
#endif

    // if there were previous raisers, get the previous raiser's stats :
    std::shared_ptr<Player> previousRaiser = getPlayerByUniqueId(opponentId);
    PreflopStatistics previousRaiserStats = opponentStats;

    if (currentHand->getPreflopRaisesNumber() > 1)
    {

        PlayerList players = currentHand->getActivePlayerList();

        for (PlayerListIterator it = players->begin(); it != players->end(); ++it)
        {

            if ((*it)->getID() == opponentId)
                continue;

            const std::vector<PlayerAction>& actions = (*it)->getCurrentHandActions().getPreflopActions();

            if (find(actions.begin(), actions.end(), PLAYER_ACTION_RAISE) != actions.end() ||
                (*it)->getAction() == PLAYER_ACTION_ALLIN)
            {
                previousRaiser = getPlayerByUniqueId((*it)->getID());
            }
        }
        previousRaiserStats = previousRaiser->getStatistics(nbPlayers).getPreflopStatistics();

        // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more
        // accurate
        if (previousRaiserStats.m_hands < MIN_HANDS_STATISTICS_ACCURATE && nbPlayers < 10 &&
            previousRaiser->getStatistics(nbPlayers + 1).getPreflopStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE)
            previousRaiserStats = previousRaiser->getStatistics(nbPlayers + 1).getPreflopStatistics();

#ifdef LOG_POKER_EXEC
        cout << "The raiser before " << opponent->getName() << " was " << previousRaiser->getName()
             << ", hands : " << previousRaiserStats.m_hands << ", " << previousRaiserStats.getVoluntaryPutMoneyInPot()
             << " / " << previousRaiserStats.getPreflopRaise() << endl
             << "\t\t";
#endif
    }

    if (opponentStats.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
    {

        if (currentHand->getPreflopRaisesNumber() == 1)
            range = opponentStats.getPreflopRaise();
        else
        {

            // there was a previous raiser, assume that the opponent has adapted his raising range to him
            if (previousRaiserStats.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
            {

                if (currentHand->getPreflopRaisesNumber() == 2)
                    range = previousRaiserStats.getPreflopRaise() * 0.7;
                else if (currentHand->getPreflopRaisesNumber() == 3)
                    range = previousRaiserStats.getPreflop3Bet() * 0.7;
                else if (currentHand->getPreflopRaisesNumber() > 3)
                    range = previousRaiserStats.getPreflop4Bet() / (currentHand->getPreflopRaisesNumber() / 2);
            }
            else
            {
                if (currentHand->getPreflopRaisesNumber() == 2)
                    range = opponentStats.getPreflopRaise();
                else if (currentHand->getPreflopRaisesNumber() == 3)
                    range = opponentStats.getPreflop3Bet();
                else if (currentHand->getPreflopRaisesNumber() > 3)
                    range = opponentStats.getPreflop4Bet() / (currentHand->getPreflopRaisesNumber() / 2);
            }
        }
    }
    else
    {

        range = getStandardRaisingRange(nbPlayers);

#ifdef LOG_POKER_EXEC
        cout << ", but not enough hands -> getting the standard range : " << range << endl << "\t\t";
        ;
#endif
        if (currentHand->getPreflopRaisesNumber() == 2)
            range = range * 0.3;
        else if (currentHand->getPreflopRaisesNumber() == 3)
            range = range * 0.2;
        else if (currentHand->getPreflopRaisesNumber() > 3)
            range = range * 0.1;
    }

#ifdef LOG_POKER_EXEC
    cout << "range is " << range;
#endif

    if (nbPlayers > 3 && previousRaiser->getID() == opponentId)
    { // adjust roughly the range giving the player's position, if there was no previous raiser

        if (opponent->getPosition() == UTG || opponent->getPosition() == UTG_PLUS_ONE ||
            opponent->getPosition() == UTG_PLUS_TWO)
            range = range * 0.9;
        else if (opponent->getPosition() == BUTTON || opponent->getPosition() == CUTOFF)
            range = range * 1.5;

#ifdef LOG_POKER_EXEC
        cout << ", position adjusted range is " << range << endl << "\t\t";
        ;
#endif
    }

    // if the player is being loose or agressive for 8 hands or so, adjust the range
    if (opponent->isInVeryLooseMode())
    {
        if (range < 40)
        {
            range = 40;
#ifdef LOG_POKER_EXEC
            cout << "\t\toveragression detected, setting range to " << range << endl;
#endif
        }
    }

    // add an error margin
    range++;

    range = ceil(range);

    if (range < 1)
        range = 1;

    if (range > 100)
        range = 100;

#ifdef LOG_POKER_EXEC
    cout << endl << "\t\testimated range is " << range << " % ";
#endif
    return getStringRange(nbPlayers, range);
}

string Player::computeEstimatedPreflopRangeFromCaller(const int opponentId, PreflopStatistics& opponentStats) const
{

    using std::cout;
    std::shared_ptr<Player> opponent = getPlayerByUniqueId(opponentId);
    bool isTopRange = true;
    std::vector<string> ranges;
    std::vector<float> rangesValues;
    const int lastRaiserID = currentHand->getPreflopLastRaiserID();
    int opponentRaises = 0;
    int opponentCalls = 0;
    const int nbPlayers = currentHand->getActivePlayerList()->size();
    float range = 0;

    for (std::vector<PlayerAction>::const_iterator i = opponent->getCurrentHandActions().getPreflopActions().begin();
         i != opponent->getCurrentHandActions().getPreflopActions().end(); i++)
    {
        if (*i == PLAYER_ACTION_RAISE || *i == PLAYER_ACTION_ALLIN)
            opponentRaises++;
        else if (*i == PLAYER_ACTION_CALL)
            opponentCalls++;
    }

    float estimatedStartingRange = opponentStats.getVoluntaryPutMoneyInPot();

    if (opponentStats.m_hands < MIN_HANDS_STATISTICS_ACCURATE)
    { // not enough hands, assume the opponent is an average tight player
        estimatedStartingRange = getStandardCallingRange(nbPlayers);
#ifdef LOG_POKER_EXEC
        cout << " [ not enough hands, getting the standard calling range ] ";
#endif
    }

#ifdef LOG_POKER_EXEC
    cout << " estimated starting range : " << estimatedStartingRange << endl << "\t\t";
    ;
#endif

    if (nbPlayers > 3)
    { // adjust roughly the range giving the player's position

        if (opponent->getPosition() == UTG || opponent->getPosition() == UTG_PLUS_ONE ||
            opponent->getPosition() == UTG_PLUS_TWO)
            estimatedStartingRange = estimatedStartingRange * 0.9;
        else if (opponent->getPosition() == BUTTON || opponent->getPosition() == CUTOFF)
            estimatedStartingRange = estimatedStartingRange * 1.4;
    }

    if (estimatedStartingRange > 100)
        estimatedStartingRange = 100;

#ifdef LOG_POKER_EXEC
    cout << ", position adjusted starting range : " << estimatedStartingRange << endl << "\t\t";
    ;
#endif

    // adjust roughly, given the pot odd the player had preflop
    const int potOdd = opponent->getPreflopPotOdd();

    if (potOdd > 70 && potOdd < 85)
        estimatedStartingRange = estimatedStartingRange * 0.7;
    else if (potOdd >= 85 && potOdd < 95)
        estimatedStartingRange = estimatedStartingRange * 0.5;
    else if (potOdd >= 95 && potOdd < 99)
        estimatedStartingRange = estimatedStartingRange * 0.3;
    else if (potOdd >= 99)
        estimatedStartingRange = estimatedStartingRange * 0.1;
    else if (potOdd <= 20 && currentHand->getPreflopRaisesNumber() < 2)
        estimatedStartingRange = 40;

#ifdef LOG_POKER_EXEC
    cout << ", pot odd adjusted starting range : " << estimatedStartingRange << endl << "\t\t";
    ;
#endif

    range = estimatedStartingRange;

    if (currentHand->getPreflopRaisesNumber() == 0 && opponentStats.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
    { // limp

        if (currentHand->getRunningPlayerList()->size() > 3)
            range = estimatedStartingRange -
                    opponentStats.getPreflopRaise(); // a hand suitable to call but not to raise ? or limp for deception
        else
            range = estimatedStartingRange;

        if (range < 5)
            range = 5;

#ifdef LOG_POKER_EXEC
        cout << ", limp range : " << range << endl << "\t\t";
        ;
#endif
    }
    else

        if (currentHand->getPreflopRaisesNumber() == 1 && opponentStats.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
    {
        range = estimatedStartingRange - opponentStats.getPreflop3Bet(); // a hand suitable to call but not to 3-bet
        if (range < 1)
            range = 1;
#ifdef LOG_POKER_EXEC
        cout << ", single bet call range : " << range << endl << "\t\t";
        ;
#endif
        if (opponentStats.getVoluntaryPutMoneyInPot() - opponentStats.getPreflopRaise() > 15)
        {
            // loose player
            range = range / 2;
#ifdef LOG_POKER_EXEC
            cout << ", loose player adjusted range : " << range << endl << "\t\t";
            ;
#endif
        }
    }
    else

        if (currentHand->getPreflopRaisesNumber() == 2)
    {

        if (opponentStats.m_hands < MIN_HANDS_STATISTICS_ACCURATE)
        { // not enough hands, assume the opponent is an average tight player
            range = estimatedStartingRange / 3;
#ifdef LOG_POKER_EXEC
            cout << ", 3-bet call range : " << range;
#endif
        }
        else
        {

            if (opponentRaises == 1)
            {
                // if the player is being 3-betted
                range = opponentStats.getPreflopRaise() * opponentStats.getPreflopCall3BetsFrequency() / 100;

                // assume that the player will adapt his calling range to the raiser's 3-bet range
                std::shared_ptr<Player> lastRaiser = getPlayerByUniqueId(lastRaiserID);
                PreflopStatistics lastRaiserStats = lastRaiser->getStatistics(nbPlayers).getPreflopStatistics();

                // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be
                // more accurate
                if (lastRaiserStats.m_hands < MIN_HANDS_STATISTICS_ACCURATE && nbPlayers < 10 &&
                    lastRaiser->getStatistics(nbPlayers + 1).getPreflopStatistics().m_hands >
                        MIN_HANDS_STATISTICS_ACCURATE)
                    lastRaiserStats = opponent->getStatistics(nbPlayers + 1).getPreflopStatistics();

                if (lastRaiserStats.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
                {
                    if (range < lastRaiserStats.getPreflop3Bet() * 0.8)
                        range = lastRaiserStats.getPreflop3Bet() * 0.8;
                }

#ifdef LOG_POKER_EXEC
                cout << ", 3-bet call range : " << range << endl << "\t\t";
                ;
#endif
            }
            else
            {
                // the player didn't raise the pot before, and there are already 2 raisers
                range = opponentStats.getVoluntaryPutMoneyInPot() / 3;
#ifdef LOG_POKER_EXEC
                cout << ", 3-bet cold-call range : " << opponentStats.getVoluntaryPutMoneyInPot() << " / 3 = " << range
                     << endl
                     << "\t\t";
                ;
#endif
            }
        }
    }
    else

        if (currentHand->getPreflopRaisesNumber() > 2)
    {

        if (opponentStats.m_hands < MIN_HANDS_STATISTICS_ACCURATE)
        { // not enough hands, assume the opponent is an average tight player
            range = estimatedStartingRange / 5;
#ifdef LOG_POKER_EXEC
            cout << ", 4-bet call range : " << range;
#endif
        }
        else
        {

            if (opponentRaises > 0)
            {
                // if the player is facing a 4-bet, after having bet
                range = (opponentStats.getPreflop3Bet() * opponentStats.getPreflopCall3BetsFrequency() / 100);
#ifdef LOG_POKER_EXEC
                cout << ", 4-bet call range : " << range << endl << "\t\t";
                ;
#endif
            }
            else
            {
                // the player didn't raise the pot before, and there are already 3 raisers
                range = opponentStats.getVoluntaryPutMoneyInPot() / 6;
#ifdef LOG_POKER_EXEC
                cout << ", 4-bet cold-call range : " << range << endl << "\t\t";
                ;
#endif
            }
        }
    }

    if (currentHand->getPreflopRaisesNumber() >= 2)
    {

        // adjust roughly, given the pot odd the player had preflop
        if (potOdd > 70 && potOdd < 85)
            range = range * 0.7;
        else if (potOdd >= 85 && potOdd < 95)
            range = range * 0.5;
        else if (potOdd >= 95 && potOdd < 99)
            range = range * 0.3;
        else if (potOdd >= 99)
            range = range * 0.1;
#ifdef LOG_POKER_EXEC
        if (potOdd > 70)
            cout << ", 3-bet or more : readjusting range with the pot odd (" << potOdd << ") : range is now " << range
                 << endl
                 << "\t\t";
        ;
#endif
    }

    // if the last raiser was being loose or agressive for 8 hands or so, adjust the range for the caller of a raise
    if (opponentRaises > 0 && lastRaiserID != -1 && currentHand->getPreflopRaisesNumber() == 1)
    {

        std::shared_ptr<Player> lastRaiser = getPlayerByUniqueId(lastRaiserID);

        if (lastRaiser->isInVeryLooseMode())
        {
            if (nbPlayers > 6 && range < 20)
            {
                range = 20;
            }
            else if (nbPlayers > 4 && range < 30)
            {
                range = 30;
            }
            else if (nbPlayers <= 4 && range < 40)
            {
                range = 40;
            }
#ifdef LOG_POKER_EXEC
            cout << "\t\toveragression detected from the raiser, setting calling range to " << range << endl;
#endif
        }
    }

    // add an error margin
    range++;

    if (range < 1)
        range = 1;

    if (range > 100)
        range = 100;

    if (potOdd < 75 && opponentStats.m_hands >= MIN_HANDS_STATISTICS_ACCURATE &&
        opponentStats.getVoluntaryPutMoneyInPot() - opponentStats.getPreflopRaise() < 10 &&
        opponentStats.getPreflopRaise() > 5 && nbPlayers > 2)
    {

        if (opponentRaises == 0 && currentHand->getPreflopRaisesNumber() == 0)
        {
            // the opponent limped
            ranges.push_back(",QJo,JTo,T9o,98o,87o,76o,65o,QTo,KJo,KTo,");
            rangesValues.push_back(9);
            ranges.push_back(SUITED_CONNECTORS + ",QTs,J9s,");
            rangesValues.push_back(SUITED_CONNECTORS_RANGE_VALUE + 0.6);
            ranges.push_back(LOW_PAIRS);
            rangesValues.push_back(LOW_PAIRS_RANGE_VALUE);
            ranges.push_back(LOW_SUITED_ACES);
            rangesValues.push_back(LOW_SUITED_ACES_RANGE_VALUE);
            ranges.push_back(LOW_OFFSUITED_ACES);
            rangesValues.push_back(LOW_OFFSUITED_ACES_RANGE_VALUE);
            ranges.push_back(HIGH_OFFSUITED_CONNECTORS);
            rangesValues.push_back(HIGH_OFFSUITED_CONNECTORS_RANGE_VALUE);
            ranges.push_back(SUITED_ONE_GAPED);
            rangesValues.push_back(SUITED_ONE_GAPED_RANGE_VALUE);
            ranges.push_back(OFFSUITED_ONE_GAPED);
            rangesValues.push_back(OFFSUITED_ONE_GAPED_RANGE_VALUE);
            ranges.push_back(SUITED_TWO_GAPED);
            rangesValues.push_back(SUITED_TWO_GAPED_RANGE_VALUE);
            ranges.push_back(SUITED_BROADWAYS);
            rangesValues.push_back(SUITED_BROADWAYS_RANGE_VALUE);
            isTopRange = false;
        }
        else if (opponentRaises == 0 && currentHand->getPreflopRaisesNumber() == 1 &&
                 opponentStats.getPreflop3Bet() > 0)
        {
            // the opponent called a single standard bet
            ranges.push_back(",77,88,99,87s,98s,T9s,JTs,");
            rangesValues.push_back(2.6);
            ranges.push_back(LOW_PAIRS);
            rangesValues.push_back(LOW_PAIRS_RANGE_VALUE);
            ranges.push_back(HIGH_OFFSUITED_CONNECTORS);
            rangesValues.push_back(HIGH_OFFSUITED_CONNECTORS_RANGE_VALUE);
            ranges.push_back(",66,TT,JJ,");
            rangesValues.push_back(1.4);
            ranges.push_back(SUITED_BROADWAYS);
            rangesValues.push_back(SUITED_BROADWAYS_RANGE_VALUE);
            ranges.push_back(",87s,98s,T9s,");
            rangesValues.push_back(0.9);
            ranges.push_back(",AQo,AJo,KQo,");
            rangesValues.push_back(2.7);
            ranges.push_back(LOW_SUITED_ACES);
            rangesValues.push_back(LOW_SUITED_ACES_RANGE_VALUE);
            ranges.push_back(HIGH_PAIRS);
            rangesValues.push_back(HIGH_PAIRS_RANGE_VALUE); // hiding a strong hand is possible
            ranges.push_back("AKo");
            rangesValues.push_back(0.9);
            ranges.push_back(HIGH_SUITED_ACES);
            rangesValues.push_back(HIGH_SUITED_ACES_RANGE_VALUE);
            ranges.push_back(SUITED_ONE_GAPED);
            rangesValues.push_back(SUITED_ONE_GAPED_RANGE_VALUE);
            ranges.push_back(OFFSUITED_ONE_GAPED);
            rangesValues.push_back(OFFSUITED_ONE_GAPED_RANGE_VALUE);
            ranges.push_back(SUITED_TWO_GAPED);
            rangesValues.push_back(SUITED_TWO_GAPED_RANGE_VALUE);
            ranges.push_back(OFFSUITED_BROADWAYS);
            rangesValues.push_back(OFFSUITED_BROADWAYS_RANGE_VALUE);
            isTopRange = false;
        }
    }

    range = ceil(range);

#ifdef LOG_POKER_EXEC
    cout << endl << "\t\testimated range is " << range << " % ";
#endif

    if (!isTopRange)
    {
#ifdef LOG_POKER_EXEC
        cout << " [ not a top range ] ";
#endif
        return getFilledRange(ranges, rangesValues, range);
    }
    else
        return getStringRange(nbPlayers, range);
}

string Player::getFilledRange(std::vector<string>& ranges, std::vector<float>& rangesValues, const float rangeMax) const
{

    float remainingRange = rangeMax;
    string estimatedRange;

    for (int i = 0; i < ranges.size(); i++)
    {
        if (remainingRange > 0)
        {
            estimatedRange += ranges.at(i);
            remainingRange -= rangesValues.at(i);
        }
    }
    if (remainingRange > 0)
    {
        // there was not enough ranges to fill
#ifdef LOG_POKER_EXEC
        cout << endl << "\t\t[ warning : not enough ranges to fill " << rangeMax << "%, setting a top range ]" << endl;
#endif
        const int nbPlayers = currentHand->getActivePlayerList()->size();
        return getStringRange(nbPlayers, rangeMax);
    }
    else
        return estimatedRange;
}

bool isDrawingProbOk(const PostFlopState& postFlopState, const int potOdd)
{

    int implicitOdd = getImplicitOdd(postFlopState);
    int drawingProb = getDrawingProbability(postFlopState);

    if (drawingProb > 0)
    {

#ifdef LOG_POKER_EXEC
        cout << endl
             << "\t\tProbability to hit a draw : " << drawingProb << "%, implicit odd : " << implicitOdd
             << ", pot odd : " << potOdd;
#endif

        if (drawingProb + implicitOdd >= potOdd)
        {
#ifdef LOG_POKER_EXEC
            cout << " - Drawing prob is ok : " << drawingProb << " + " << implicitOdd << " > " << potOdd << endl;
#endif
            return true;
        }
#ifdef LOG_POKER_EXEC
        else
            cout << " - Drawing prob is not ok" << endl;
#endif
    }
    return false;
}

bool Player::isPreflopBigBet() const
{

    if (getPotOdd() > 70)
        return true;

    const int highestSet = min(myCash, currentHand->getCurrentBettingRound()->getHighestSet());

    if (highestSet > currentHand->getSmallBlind() * 8 && highestSet - mySet > mySet * 6)
        return true;

    return false;
}

bool Player::isAgressor(const GameState gameState) const
{

    if (gameState == GAME_STATE_PREFLOP && currentHand->getPreflopLastRaiserID() == myID)
        return true;

    if (gameState == GAME_STATE_FLOP && currentHand->getFlopLastRaiserID() == myID)
        return true;

    if (gameState == GAME_STATE_TURN && currentHand->getTurnLastRaiserID() == myID)
        return true;

    if (gameState == GAME_STATE_RIVER && currentHand->getLastRaiserID() == myID)
        return true;

    return false;
}
bool Player::canBluff(const GameState gameState) const
{

    // check if there is no calling station at the table
    // check also if my opponents stacks are big enough to bluff them

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    const int nbRaises = currentHand->getPreflopRaisesNumber();

    PlayerList players = currentHand->getRunningPlayerList();

    if (players->size() == 1)
        // all other players are allin
        return false;

    for (PlayerListIterator it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getID() == myID)
            continue;

        PreflopStatistics preflopStats = (*it)->getStatistics(nbPlayers).getPreflopStatistics();

        // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more
        // accurate
        if (preflopStats.m_hands < MIN_HANDS_STATISTICS_ACCURATE && nbPlayers < 10 &&
            (*it)->getStatistics(nbPlayers + 1).getPreflopStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE)

            preflopStats = (*it)->getStatistics(nbPlayers + 1).getPreflopStatistics();

        if ((*it)->getStatistics(nbPlayers).getWentToShowDown() >= 40 &&
            preflopStats.getVoluntaryPutMoneyInPot() - preflopStats.getPreflopRaise() > 15 &&
            preflopStats.getVoluntaryPutMoneyInPot() > 20)
            return false; // seems to be a calling station

        if ((*it)->getCash() < currentHand->getBoard()->getPot() * 3)
            return false;

        if (gameState == GAME_STATE_PREFLOP)
        {
            if (preflopStats.getPreflopCall3BetsFrequency() > 40)
                return false;
        }
    }

    return true;
}

std::string getStringRange(int nbPlayers, int range)
{

    if (range > 100)
    { // should never happen, but...
        cout << "warning : bad range in getStringRange : " << range << endl;
        range = 100;
    }

    if (nbPlayers == 2)
        return TOP_RANGE_2_PLAYERS[range];
    else if (nbPlayers == 3)
        return TOP_RANGE_3_PLAYERS[range];
    else if (nbPlayers == 4)
        return TOP_RANGE_4_PLAYERS[range];
    else
        return TOP_RANGE_MORE_4_PLAYERS[range];
}

int Player::getPreflopPotOdd() const
{
    return myPreflopPotOdd;
}

int Player::getStandardRaisingRange(int nbPlayers) const
{

    if (nbPlayers == 2)
        return 39;
    else if (nbPlayers == 3)
        return 36;
    else if (nbPlayers == 4)
        return 33;
    else if (nbPlayers == 5)
        return 30;
    else if (nbPlayers == 6)
        return 27;
    else if (nbPlayers == 7)
        return 24;
    else if (nbPlayers == 8)
        return 21;
    else if (nbPlayers == 9)
        return 18;
    else
        return 15;
}
int Player::getStandardCallingRange(int nbPlayers) const
{

    return getStandardRaisingRange(nbPlayers) + 5;
}

void Player::setPreflopPotOdd(const int potOdd)
{
    myPreflopPotOdd = potOdd;
}

std::string Player::getEstimatedRange() const
{
    return myEstimatedRange;
}
void Player::setEstimatedRange(const std::string range)
{
    myEstimatedRange = range;
}

bool Player::isInVeryLooseMode() const
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);

    PreflopStatistics preflop = stats.getPreflopStatistics();

    if (preflop.GetLastActionsNumber(PLAYER_ACTION_ALLIN) + preflop.GetLastActionsNumber(PLAYER_ACTION_RAISE) +
            preflop.GetLastActionsNumber(PLAYER_ACTION_CALL) >
        PreflopStatistics::LAST_ACTIONS_STACK_SIZE * 0.8)
        return true;
    else
        return false;
}

} // namespace pkt::core
