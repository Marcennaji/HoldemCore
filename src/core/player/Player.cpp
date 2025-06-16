// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Player.h"

#include <core/engine/CardsValue.h>
#include <core/engine/Randomizer.h>
#include <core/engine/model/Ranges.h>
#include <core/interfaces/IHand.h>
#include <core/interfaces/ILogger.h>
#include <core/interfaces/persistence/IHandAuditStore.h>
#include <core/interfaces/persistence/IPlayersStatisticsStore.h>
#include <core/player/range/RangeParser.h>
#include <core/player/strategy/CurrentHandContext.h>
#include <third_party/psim/psim.hpp>
#include "Helpers.h"

#include <sstream>

namespace pkt::core::player
{

using namespace std;

Player::Player(GameEvents* events, ILogger* logger, IHandAuditStore* ha, IPlayersStatisticsStore* ps, int id,
               std::string name, int sC, bool aS, int mB)
    : myHandAuditStore(ha), myPlayersStatisticsStore(ps), currentHand(0), myID(id), myName(name), myCardsValueInt(0),
      myCash(sC), mySet(0), myLastRelativeSet(0), myAction(PLAYER_ACTION_NONE), myButton(mB), myActiveStatus(aS),
      myTurn(0), myCardsFlip(0), myRoundStartCash(0), lastMoneyWon(0), m_isSessionActive(false), myEvents(events),
      myLogger(logger)
{
    myRangeEstimator = std::make_unique<RangeEstimator>(myID, myPlayersStatisticsStore, myLogger);
    myCurrentHandContext = std::make_unique<CurrentHandContext>();
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

void Player::setHand(IHand* h)
{
    currentHand = h;
    myRangeEstimator->setHand(h);
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
        myLogger->error("Pot = " + std::to_string(currentHand->getBoard()->getPot()) + " + " +
                        std::to_string(currentHand->getBoard()->getSets()) + " = " + std::to_string(pot));
        return 0;
    }

    int odd = (highestSet - mySet) * 100 / pot;
    if (odd < 0)
        odd = -odd; // happens if mySet > highestSet

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

    std::ostringstream logMessage;
    logMessage << "\n"
               << "\tsimulation with " << cards << " : " << std::endl
               << "\t\twin at showdown is " << r.winSd << std::endl
               << "\t\ttie at showdown is " << r.tieSd << std::endl
               << "\t\twin now (against random hands) is " << r.win << std::endl
               << "\t\twin now (against ranged hands) is " << r.winRanged << std::endl;

    if (r.winRanged == 0)
    {
        logMessage << "\n"
                   << "\t\tr.winRanged = 1 - " << maxOpponentsStrengths
                   << " = 0 : setting value to r.win / 4 = " << r.win / 4 << std::endl;
    }

    if (getPotOdd() > 0)
    {
        logMessage << "\t\tpot odd is " << getPotOdd() << std::endl;
    }

    myLogger->info(logMessage.str());
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

    if (opponent->myRangeEstimator->getEstimatedRange().size() == 0)
    {
        opponent->getRangeEstimator()->computeEstimatedPreflopRange(*myCurrentHandContext);
    }

    vector<std::string> ranges = RangeParser::getRangeAtomicValues(opponent->myRangeEstimator->getEstimatedRange());

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
    if (!isCardsInRange(myCard1, myCard2, myRangeEstimator->getEstimatedRange()))
    {
        std::cout << endl
                  << "\t" << myCard1 << " " << myCard2 << " isn't part of the plausible " << label << " range :\t"
                  << myRangeEstimator->getEstimatedRange() << endl;
        currentHand->getHandAuditStore()->updateUnplausibleHand(myCard1, myCard2, (myID == 0 ? true : false),
                                                                bettingRound, nbPlayers);
    }
    else
        std::cout << endl
                  << "\t" << myCard1 << " " << myCard2 << " is part of the plausible " << label << " range :\t"
                  << myRangeEstimator->getEstimatedRange() << endl;
}

std::map<int, float> Player::evaluateOpponentsStrengths() const
{

    map<int, float> result;
    PlayerList players = currentHand->getActivePlayerList();
    const int nbPlayers = currentHand->getActivePlayerList()->size();

    for (PlayerListIterator it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getID() == myID || (*it)->getAction() == PLAYER_ACTION_FOLD ||
            (*it)->getAction() == PLAYER_ACTION_NONE)
            continue;

        const float estimatedOpponentWinningHands = getOpponentWinningHandsPercentage((*it)->getID(), getStringBoard());
        assert(estimatedOpponentWinningHands <= 1.0);

        result[(*it)->getID()] = estimatedOpponentWinningHands;
    }

    return result;
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

void Player::updateCurrentHandContext(const GameState state)
{

    // general (and shared) game state
    myCurrentHandContext->gameState = state;
    myCurrentHandContext->nbRunningPlayers = currentHand->getRunningPlayerList()->size();
    myCurrentHandContext->lastVPIPPlayer = getPlayerByUniqueId(currentHand->getLastRaiserID());
    myCurrentHandContext->callersPositions = currentHand->getCallersPositions();
    myCurrentHandContext->pot = currentHand->getBoard()->getPot();
    myCurrentHandContext->potOdd = getPotOdd();
    myCurrentHandContext->sets = currentHand->getBoard()->getSets();
    myCurrentHandContext->highestSet = currentHand->getCurrentBettingRound()->getHighestSet();
    myCurrentHandContext->stringBoard = getStringBoard();
    myCurrentHandContext->preflopLastRaiser = getPlayerByUniqueId(currentHand->getPreflopLastRaiserID());
    myCurrentHandContext->preflopRaisesNumber = currentHand->getPreflopRaisesNumber();
    myCurrentHandContext->preflopCallsNumber = currentHand->getPreflopCallsNumber();
    myCurrentHandContext->isPreflopBigBet = isPreflopBigBet();
    myCurrentHandContext->flopBetsOrRaisesNumber = currentHand->getFlopBetsOrRaisesNumber();
    myCurrentHandContext->flopLastRaiser = getPlayerByUniqueId(currentHand->getFlopLastRaiserID());
    myCurrentHandContext->turnBetsOrRaisesNumber = currentHand->getTurnBetsOrRaisesNumber();
    myCurrentHandContext->turnLastRaiser = getPlayerByUniqueId(currentHand->getTurnLastRaiserID());
    myCurrentHandContext->riverBetsOrRaisesNumber = currentHand->getRiverBetsOrRaisesNumber();
    myCurrentHandContext->nbPlayers = currentHand->getActivePlayerList()->size();
    myCurrentHandContext->smallBlind = currentHand->getSmallBlind();

    for (std::vector<PlayerAction>::const_iterator i = myCurrentHandActions.m_flopActions.begin();
         i != myCurrentHandActions.m_flopActions.end(); i++)
    {

        if (*i == PLAYER_ACTION_RAISE)
            myCurrentHandContext->nbRaises++;
        else if (*i == PLAYER_ACTION_BET)
            myCurrentHandContext->nbBets++;
        else if (*i == PLAYER_ACTION_CHECK)
            myCurrentHandContext->nbChecks++;
        else if (*i == PLAYER_ACTION_CALL)
            myCurrentHandContext->nbCalls++;
        else if (*i == PLAYER_ACTION_ALLIN)
            myCurrentHandContext->nbAllins++;
    }

    // Player-specific, visible from the opponents :
    myCurrentHandContext->myCash = myCash;
    myCurrentHandContext->mySet = mySet;
    myCurrentHandContext->myM = static_cast<int>(getM());
    myCurrentHandContext->myCurrentHandActions = myCurrentHandActions;
    myCurrentHandContext->myHavePosition = Player::getHavePosition(myPosition, currentHand->getRunningPlayerList());
    myCurrentHandContext->myPreflopIsAggressor = isAgressor(GAME_STATE_PREFLOP);
    myCurrentHandContext->myFlopIsAggressor = isAgressor(GAME_STATE_FLOP);
    myCurrentHandContext->myTurnIsAggressor = isAgressor(GAME_STATE_TURN);
    myCurrentHandContext->myRiverIsAggressor = isAgressor(GAME_STATE_RIVER);
    myCurrentHandContext->myStatistics = getStatistics(currentHand->getActivePlayerList()->size());
    myCurrentHandContext->myID = myID;
    myCurrentHandContext->myIsInVeryLooseMode = isInVeryLooseMode(currentHand->getActivePlayerList()->size());
    myCurrentHandContext->myPosition = myPosition;

    // player specific, hidden from the opponents :
    myCurrentHandContext->myCanBluff = canBluff(state);
    myCurrentHandContext->myPreflopCallingRange = calculatePreflopCallingRange(*myCurrentHandContext);
    myCurrentHandContext->myHandSimulation = getHandSimulation();
    myCurrentHandContext->myCard1 = myCard1;
    myCurrentHandContext->myCard2 = myCard2;
    myCurrentHandContext->myPostFlopState = getPostFlopState();
}

float Player::calculatePreflopCallingRange(CurrentHandContext& context, bool deterministic) const
{
    return myRangeEstimator->getStandardCallingRange(currentHand->getActivePlayerList()->size());
}

} // namespace pkt::core::player
