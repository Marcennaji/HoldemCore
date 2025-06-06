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

#include <sstream>

namespace pkt::core::player
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
    myRangeManager = std::make_unique<RangeManager>(myID, myPlayersStatisticsStore);
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
    myRangeManager->setHand(br);
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
    if (myPlayersStatisticsStore)
        myStatistics = myPlayersStatisticsStore->getPlayerStatistics(myName);
    else
        myStatistics.fill(PlayerStatistics());
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
    //		if (opponent->isInVeryLooseMode(nbPlayers))
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

    vector<std::string> ranges = myRangeManager->getRangeAtomicValues(opponent->getEstimatedRange());

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
    const string originalEstimatedRange = myRangeManager->getEstimatedRange();

#ifdef LOG_POKER_EXEC
    std::cout << endl
              << "\tPlausible range on preflop for player " << myID << " :\t" << myRangeManager->getEstimatedRange()
              << endl;
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
            myRangeManager->setEstimatedRange(myRangeManager->substractRange(
                myRangeManager->getEstimatedRange(), RangeManager::getStringRange(nbPlayers, stats.getPreflopRaise())));
        else
            myRangeManager->setEstimatedRange(myRangeManager->substractRange(
                myRangeManager->getEstimatedRange(),
                RangeManager::getStringRange(nbPlayers, myRangeManager->getStandardRaisingRange(nbPlayers))));
    }

    if (myRangeManager->getEstimatedRange() == "")
        myRangeManager->setEstimatedRange(originalEstimatedRange);

#ifdef LOG_POKER_EXEC
    logUnplausibleHands(GAME_STATE_PREFLOP);
#endif
}

// purpose : remove some unplausible hands, would would normally be in the estimated preflop range, given what I did on
// flop
void Player::updateUnplausibleRangesGivenFlopActions()
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);

    const string originalEstimatedRange = myRangeManager->getEstimatedRange();
    string unplausibleRanges;

#ifdef LOG_POKER_EXEC
    std::cout << endl << "\tPlausible range on flop, before update :\t" << myRangeManager->getEstimatedRange() << endl;
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

    if (isInVeryLooseMode(nbPlayers))
    {
#ifdef LOG_POKER_EXEC
        std::cout << endl
                  << "\tSeems to be (temporarily ?) on very loose mode : estimated range is\t"
                  << myRangeManager->getEstimatedRange() << endl;
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

    vector<std::string> ranges = myRangeManager->getRangeAtomicValues(myRangeManager->getEstimatedRange());

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

    myRangeManager->setEstimatedRange(
        myRangeManager->substractRange(myRangeManager->getEstimatedRange(), unplausibleRanges, stringBoard));

    if (myRangeManager->getEstimatedRange() == "")
    {
        // keep previous range
#ifdef LOG_POKER_EXEC
        cout << "\tCan't remove all plausible ranges, keeping last one" << endl;
#endif
        myRangeManager->setEstimatedRange(originalEstimatedRange);
        unplausibleRanges = "";
    }

#ifdef LOG_POKER_EXEC
    if (unplausibleRanges != "")
        cout << "\tRemoving unplausible ranges : " << unplausibleRanges << endl;
    logUnplausibleHands(GAME_STATE_FLOP);
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

    if (isInVeryLooseMode(nbPlayers))
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

    if (isInVeryLooseMode(nbPlayers))
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

    if (isInVeryLooseMode(nbPlayers))
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

    if (isInVeryLooseMode(nbPlayers))
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
    const string originalEstimatedRange = myRangeManager->getEstimatedRange();
    string unplausibleRanges;

#ifdef LOG_POKER_EXEC
    std::cout << endl << "\tPlausible range on turn, before update :\t" << myRangeManager->getEstimatedRange() << endl;
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

    if (isInVeryLooseMode(nbPlayers))
    {
#ifdef LOG_POKER_EXEC
        std::cout << endl
                  << "\tSeems to be (temporarily ?) on very loose mode : estimated range is\t"
                  << myRangeManager->getEstimatedRange() << endl;
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

    vector<std::string> ranges = myRangeManager->getRangeAtomicValues(myRangeManager->getEstimatedRange());

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

    myRangeManager->setEstimatedRange(
        myRangeManager->substractRange(myRangeManager->getEstimatedRange(), unplausibleRanges, stringBoard));

    if (myRangeManager->getEstimatedRange() == "")
    {
        // keep previous range
#ifdef LOG_POKER_EXEC
        cout << "\tCan't remove all plausible ranges, keeping last one" << endl;
#endif
        myRangeManager->setEstimatedRange(originalEstimatedRange);
        unplausibleRanges = "";
    }

#ifdef LOG_POKER_EXEC
    if (unplausibleRanges != "")
        cout << "\tRemoving unplausible ranges : " << unplausibleRanges << endl;
    logUnplausibleHands(GAME_STATE_TURN);
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

    if (isInVeryLooseMode(nbPlayers))
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

    if (isInVeryLooseMode(nbPlayers))
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

    if (isInVeryLooseMode(nbPlayers))
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

    if (isInVeryLooseMode(nbPlayers))
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
    const string originalEstimatedRange = myRangeManager->getEstimatedRange();
    string unplausibleRanges;

#ifdef LOG_POKER_EXEC
    std::cout << endl << "\tPlausible range on river, before update :\t" << myRangeManager->getEstimatedRange() << endl;
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

    if (isInVeryLooseMode(nbPlayers))
    {
#ifdef LOG_POKER_EXEC
        std::cout << endl
                  << "\tSeems to be on very loose mode : estimated range is\t" << myRangeManager->getEstimatedRange()
                  << endl;
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

    vector<std::string> ranges = myRangeManager->getRangeAtomicValues(myRangeManager->getEstimatedRange());

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

    myRangeManager->setEstimatedRange(
        myRangeManager->substractRange(myRangeManager->getEstimatedRange(), unplausibleRanges, stringBoard));

    if (myRangeManager->getEstimatedRange() == "")
    {
        // keep previous range
#ifdef LOG_POKER_EXEC
        cout << "\tCan't remove all plausible ranges, keeping last one" << endl;
#endif
        myRangeManager->setEstimatedRange(originalEstimatedRange);
        unplausibleRanges = "";
    }

#ifdef LOG_POKER_EXEC
    if (unplausibleRanges != "")
        cout << "\tRemoving unplausible ranges : " << unplausibleRanges << endl;
    logUnplausibleHands(GAME_STATE_RIVER);
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

    if (isInVeryLooseMode(nbPlayers))
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

    if (isInVeryLooseMode(nbPlayers))
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

    if (isInVeryLooseMode(nbPlayers))
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

    if (isInVeryLooseMode(nbPlayers))
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    // the player has raised twice or more the river, and is not a maniac player : he should have at least a straight
    if (nbRaises > 1 && !(r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsQuads || r.IsStFlush))
        return true;

    return false;
}

void Player::logUnplausibleHands(GameState g)
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
    // during dev phase : if some hands are not part of the estimated starting range for this player, insert them in a
    // database, for auditing purposes
    if (!isCardsInRange(myCard1, myCard2, myRangeManager->getEstimatedRange()))
    {
        std::cout << endl
                  << "\t" << myCard1 << " " << myCard2 << " isn't part of the plausible " << label << " range :\t"
                  << myRangeManager->getEstimatedRange() << endl;
        currentHand->getHandAuditStore()->updateUnplausibleHand(myCard1, myCard2, (myID == 0 ? true : false),
                                                                bettingRound, nbPlayers);
    }
    else
        std::cout << endl
                  << "\t" << myCard1 << " " << myCard2 << " is part of the plausible " << label << " range :\t"
                  << myRangeManager->getEstimatedRange() << endl;
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
    const int nbPlayers = currentHand->getActivePlayerList()->size();
    std::shared_ptr<Player> opponent = getPlayerByUniqueId(opponentId);
    const int lastRaiserID = currentHand->getPreflopLastRaiserID();
    std::shared_ptr<Player> lastRaiser = getPlayerByUniqueId(lastRaiserID);
    bool looseMode = false;
    if (lastRaiserID != -1)
        looseMode = lastRaiser->isInVeryLooseMode(nbPlayers);

    myRangeManager->computeEstimatedPreflopRange(
        *opponent, nbPlayers, lastRaiserID, currentHand->getPreflopRaisesNumber(),
        getPreviousRaiserStats(opponentId, opponent->getStatistics(nbPlayers).getPreflopStatistics()), looseMode);
}

const PreflopStatistics Player::getPreviousRaiserStats(const int opponentId,
                                                       const PreflopStatistics& opponentStats) const
{
    const int nbPlayers = currentHand->getActivePlayerList()->size();
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

        // If not enough hands, try to use statistics for (nbPlayers + 1)
        if (previousRaiserStats.m_hands < MIN_HANDS_STATISTICS_ACCURATE && nbPlayers < 10 &&
            previousRaiser->getStatistics(nbPlayers + 1).getPreflopStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        {
            previousRaiserStats = previousRaiser->getStatistics(nbPlayers + 1).getPreflopStatistics();
        }

#ifdef LOG_POKER_EXEC
        std::cout << "The raiser before " << getPlayerByUniqueId(opponentId)->getName() << " was "
                  << previousRaiser->getName() << ", hands: " << previousRaiserStats.m_hands << ", "
                  << previousRaiserStats.getVoluntaryPutMoneyInPot() << " / " << previousRaiserStats.getPreflopRaise()
                  << std::endl
                  << "\t\t";
#endif
    }

    return previousRaiserStats;
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

int Player::getPreflopPotOdd() const
{
    return myPreflopPotOdd;
}

void Player::setPreflopPotOdd(const int potOdd)
{
    myPreflopPotOdd = potOdd;
}

std::string Player::getEstimatedRange() const
{
    return myRangeManager->getEstimatedRange();
}
void Player::setEstimatedRange(const std::string range)
{
    myRangeManager->setEstimatedRange(range);
}

bool Player::isInVeryLooseMode(const int nbPlayers) const
{
    PlayerStatistics stats = getStatistics(nbPlayers);

    PreflopStatistics preflop = stats.getPreflopStatistics();

    if (preflop.GetLastActionsNumber(PLAYER_ACTION_ALLIN) + preflop.GetLastActionsNumber(PLAYER_ACTION_RAISE) +
            preflop.GetLastActionsNumber(PLAYER_ACTION_CALL) >
        PreflopStatistics::LAST_ACTIONS_STACK_SIZE * 0.8)
        return true;
    else
        return false;
}

} // namespace pkt::core::player
