// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Player.h"

#include <core/engine/CardsValue.h>
#include <core/engine/Randomizer.h>
#include <core/engine/model/Ranges.h>
#include <core/interfaces/IHand.h>
#include <core/interfaces/persistence/IHandAuditStore.h>
#include <core/interfaces/persistence/IPlayersStatisticsStore.h>
#include <core/player/range/RangeParser.h>
#include <core/player/strategy/CurrentHandContext.h>
#include <core/services/GlobalServices.h>
#include <third_party/psim/psim.hpp>
#include "Helpers.h"

#include <sstream>

namespace pkt::core::player
{

using namespace std;

Player::Player(const GameEvents& events, int id, std::string name, int sC, bool aS, int mB)
    : currentHand(0), myID(id), myName(name), myCardsValueInt(0), myCash(sC), mySet(0), myLastRelativeSet(0),
      myAction(PlayerActionNone), myButton(mB), myActiveStatus(aS), myTurn(0), myCardsFlip(0), myRoundStartCash(0),
      lastMoneyWon(0), m_isSessionActive(false), myEvents(events)
{
    myRangeEstimator = std::make_unique<RangeEstimator>(myID);
    myCurrentHandContext = std::make_unique<CurrentHandContext>();
    loadStatistics();

    int i;
    for (i = 0; i < 2; i++)
    {
        myCards[i] = -1;
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
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, CUTOFF, MIDDLE, MIDDLE, UtgPlusOne, UtgPlusOne,
         UtgPlusOne}, // my position = dealer + 4
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, CUTOFF, LATE, MIDDLE, UtgPlusTwo,
         UtgPlusTwo}, // my position = dealer + 5
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, CUTOFF, LATE, MIDDLE,
         MIDDLE}, // my position = dealer + 6
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, CUTOFF, LATE,
         MiddlePlusOne}, // my position = dealer + 7
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
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, SB, BB, UTG, MIDDLE, MIDDLE, MiddlePlusOne,
         MiddlePlusOne}, // my position = dealer - 3
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, SB, BB, UTG, UtgPlusOne, UtgPlusTwo,
         MIDDLE}, // my position = dealer - 4
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, SB, BB, UTG, UtgPlusOne,
         UtgPlusTwo}, // my position = dealer - 5
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, SB, BB, UTG,
         UtgPlusOne}, // my position = dealer - 6
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
                if ((*it_c)->getId() == dealerPosition)
                    dealerFound = true;
                else if (dealerFound)
                    pos++;
                if ((*it_c)->getId() == myID)
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

                if ((*it_c)->getId() == myID)
                    myPositionFound = true;
                else if (myPositionFound)
                    pos++;
                if ((*it_c)->getId() == dealerPosition)
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
    case UtgPlusOne:
        return "UtgPlusOne";
        break;
    case UtgPlusTwo:
        return "UtgPlusTwo";
        break;
    case MIDDLE:
        return "MIDDLE";
        break;
    case MiddlePlusOne:
        return "MiddlePlusOne";
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

int Player::getId() const
{
    return myID;
}
CurrentHandActions& Player::getCurrentHandActions()
{
    return myCurrentHandActions;
}

void Player::setId(unsigned newId)
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
    case PlayerActionAllin:
        myStatistics[nbPlayers].m_preflopStatistics.m_raises++;
        break;
    case PlayerActionRaise:
        myStatistics[nbPlayers].m_preflopStatistics.m_raises++;
        break;
    case PlayerActionFold:
        myStatistics[nbPlayers].m_preflopStatistics.m_folds++;
        break;
    case PlayerActionCheck:
        myStatistics[nbPlayers].m_preflopStatistics.m_checks++;
        break;
    case PlayerActionCall:
        myStatistics[nbPlayers].m_preflopStatistics.m_calls++;
        break;
    default:
        break;
    }

    myStatistics[nbPlayers].m_preflopStatistics.addLastAction(myAction); // keep track of the last 10 actions

    if (myAction == PlayerActionCall && currentHand->getRaisersPositions().size() == 0) //
        myStatistics[nbPlayers].m_preflopStatistics.m_limps++;

    int playerRaises = 0;
    for (std::vector<PlayerAction>::const_iterator i = myCurrentHandActions.m_preflopActions.begin();
         i != myCurrentHandActions.m_preflopActions.end(); i++)
    {
        if (*i == PlayerActionRaise || *i == PlayerActionAllin)
            playerRaises++;
    }

    if (myAction == PlayerActionRaise || myAction == PlayerActionAllin)
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

            if (myAction == PlayerActionCall)
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
    case PlayerActionAllin:
        myStatistics[nbPlayers].m_flopStatistics.m_raises++;
        break;
    case PlayerActionRaise:
        myStatistics[nbPlayers].m_flopStatistics.m_raises++;
        break;
    case PlayerActionFold:
        myStatistics[nbPlayers].m_flopStatistics.m_folds++;
        break;
    case PlayerActionCheck:
        myStatistics[nbPlayers].m_flopStatistics.m_checks++;
        break;
    case PlayerActionCall:
        myStatistics[nbPlayers].m_flopStatistics.m_calls++;
        break;
    case PlayerActionBet:
        myStatistics[nbPlayers].m_flopStatistics.m_bets++;
        break;
    default:
        break;
    }
    if (myAction == PlayerActionRaise && currentHand->getRaisersPositions().size() > 1)
        myStatistics[nbPlayers].m_flopStatistics.m_3Bets++;

    // continuation bets
    if (currentHand->getPreflopLastRaiserId() == myID)
    {
        myStatistics[nbPlayers].m_flopStatistics.m_continuationBetsOpportunities++;
        if (myAction == PlayerActionBet)
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
    case PlayerActionAllin:
        myStatistics[nbPlayers].m_turnStatistics.m_raises++;
        break;
    case PlayerActionRaise:
        myStatistics[nbPlayers].m_turnStatistics.m_raises++;
        break;
    case PlayerActionFold:
        myStatistics[nbPlayers].m_turnStatistics.m_folds++;
        break;
    case PlayerActionCheck:
        myStatistics[nbPlayers].m_turnStatistics.m_checks++;
        break;
    case PlayerActionCall:
        myStatistics[nbPlayers].m_turnStatistics.m_calls++;
        break;
    case PlayerActionBet:
        myStatistics[nbPlayers].m_turnStatistics.m_bets++;
        break;
    default:
        break;
    }
    if (myAction == PlayerActionRaise && currentHand->getRaisersPositions().size() > 1)
        myStatistics[nbPlayers].m_turnStatistics.m_3Bets++;
}
void Player::updateRiverStatistics()
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();

    if (myCurrentHandActions.m_riverActions.size() == 1)
        myStatistics[nbPlayers].m_riverStatistics.m_hands++;

    switch (myAction)
    {
    case PlayerActionAllin:
        myStatistics[nbPlayers].m_riverStatistics.m_raises++;
        break;
    case PlayerActionRaise:
        myStatistics[nbPlayers].m_riverStatistics.m_raises++;
        break;
    case PlayerActionFold:
        myStatistics[nbPlayers].m_riverStatistics.m_folds++;
        break;
    case PlayerActionCheck:
        myStatistics[nbPlayers].m_riverStatistics.m_checks++;
        break;
    case PlayerActionCall:
        myStatistics[nbPlayers].m_riverStatistics.m_calls++;
        break;
    case PlayerActionBet:
        myStatistics[nbPlayers].m_riverStatistics.m_bets++;
        break;
    default:
        break;
    }
    if (myAction == PlayerActionRaise && currentHand->getRaisersPositions().size() > 1)
        myStatistics[nbPlayers].m_riverStatistics.m_3Bets++;
}

void Player::loadStatistics()
{

    resetPlayerStatistics(); // reset stats to 0

    myStatistics = GlobalServices::instance().playersStatisticsStore()->getPlayerStatistics(myName);
    if (myStatistics.empty())
        myStatistics.fill(PlayerStatistics());
}

const PostFlopState Player::getPostFlopState() const
{

    std::string stringHand = getCardsValueString();
    std::string stringBoard = getStringBoard();

    PostFlopState r;

    getHandState((stringHand + stringBoard).c_str(), &r);

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

    if (currentHand->getCurrentRound() == GameStateFlop)
        cardsOnBoard = 3;
    else if (currentHand->getCurrentRound() == GameStateTurn)
        cardsOnBoard = 4;
    else if (currentHand->getCurrentRound() == GameStateRiver)
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
        if ((*i)->getId() == id)
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
        GlobalServices::instance().logger()->error("Pot = " + std::to_string(currentHand->getBoard()->getPot()) +
                                                   " + " + std::to_string(currentHand->getBoard()->getSets()) + " = " +
                                                   std::to_string(pot));
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

    simulateHand(cards.c_str(), &r, 0, 1, 0);

    float win = r.win; // save the value

    const int nbOpponents = std::max(1, (int) currentHand->getRunningPlayerList()->size() -
                                            1); // note that allin opponents are not "running" any more
    simulateHandMulti(cards.c_str(), &r, 200, 100, nbOpponents);
    r.win = win; // because simulateHandMulti doesn't compute 'win'

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

    GlobalServices::instance().logger()->info(logMessage.str());
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

    const int myRank = rankHand((myCard1 + myCard2 + board).c_str());

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
            GlobalServices::instance().logger()->error("invalid hand : " + (*i));
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
        if (rankHand(((*i) + board).c_str()) > myRank)
            nbWinningHands++;
    }
    if (ranges.size() == 0)
    {
        GlobalServices::instance().logger()->error("no ranges for opponent " + std::to_string(opponentId));
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

    if (g == GameStatePreflop)
    {
        label = "preflop";
        bettingRound = 'P';
    }
    else if (g == GameStateFlop)
    {
        label = "flop";
        bettingRound = 'F';
    }
    else if (g == GameStateTurn)
    {
        label = "turn";
        bettingRound = 'T';
    }
    else if (g == GameStateRiver)
    {
        label = "river";
        bettingRound = 'R';
    }
}

std::map<int, float> Player::evaluateOpponentsStrengths() const
{

    map<int, float> result;
    PlayerList players = currentHand->getActivePlayerList();
    const int nbPlayers = currentHand->getActivePlayerList()->size();

    for (PlayerListIterator it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getId() == myID || (*it)->getAction() == PlayerActionFold || (*it)->getAction() == PlayerActionNone)
            continue;

        const float estimatedOpponentWinningHands = getOpponentWinningHandsPercentage((*it)->getId(), getStringBoard());
        assert(estimatedOpponentWinningHands <= 1.0);

        result[(*it)->getId()] = estimatedOpponentWinningHands;
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

    if (gameState == GameStatePreflop && currentHand->getPreflopLastRaiserId() == myID)
        return true;

    if (gameState == GameStateFlop && currentHand->getFlopLastRaiserId() == myID)
        return true;

    if (gameState == GameStateTurn && currentHand->getTurnLastRaiserId() == myID)
        return true;

    if (gameState == GameStateRiver && currentHand->getLastRaiserId() == myID)
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

        if ((*it)->getId() == myID)
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

        if (gameState == GameStatePreflop)
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

    if (preflop.getLastActionsNumber(PlayerActionAllin) + preflop.getLastActionsNumber(PlayerActionRaise) +
            preflop.getLastActionsNumber(PlayerActionCall) >
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
    myCurrentHandContext->lastVPIPPlayer = getPlayerByUniqueId(currentHand->getLastRaiserId());
    myCurrentHandContext->callersPositions = currentHand->getCallersPositions();
    myCurrentHandContext->pot = currentHand->getBoard()->getPot();
    myCurrentHandContext->potOdd = getPotOdd();
    myCurrentHandContext->sets = currentHand->getBoard()->getSets();
    myCurrentHandContext->highestSet = currentHand->getCurrentBettingRound()->getHighestSet();
    myCurrentHandContext->stringBoard = getStringBoard();
    myCurrentHandContext->preflopLastRaiser = getPlayerByUniqueId(currentHand->getPreflopLastRaiserId());
    myCurrentHandContext->preflopRaisesNumber = currentHand->getPreflopRaisesNumber();
    myCurrentHandContext->preflopCallsNumber = currentHand->getPreflopCallsNumber();
    myCurrentHandContext->isPreflopBigBet = isPreflopBigBet();
    myCurrentHandContext->flopBetsOrRaisesNumber = currentHand->getFlopBetsOrRaisesNumber();
    myCurrentHandContext->flopLastRaiser = getPlayerByUniqueId(currentHand->getFlopLastRaiserId());
    myCurrentHandContext->turnBetsOrRaisesNumber = currentHand->getTurnBetsOrRaisesNumber();
    myCurrentHandContext->turnLastRaiser = getPlayerByUniqueId(currentHand->getTurnLastRaiserId());
    myCurrentHandContext->riverBetsOrRaisesNumber = currentHand->getRiverBetsOrRaisesNumber();
    myCurrentHandContext->nbPlayers = currentHand->getActivePlayerList()->size();
    myCurrentHandContext->smallBlind = currentHand->getSmallBlind();

    for (std::vector<PlayerAction>::const_iterator i = myCurrentHandActions.m_flopActions.begin();
         i != myCurrentHandActions.m_flopActions.end(); i++)
    {

        if (*i == PlayerActionRaise)
            myCurrentHandContext->nbRaises++;
        else if (*i == PlayerActionBet)
            myCurrentHandContext->nbBets++;
        else if (*i == PlayerActionCheck)
            myCurrentHandContext->nbChecks++;
        else if (*i == PlayerActionCall)
            myCurrentHandContext->nbCalls++;
        else if (*i == PlayerActionAllin)
            myCurrentHandContext->nbAllins++;
    }

    // Player-specific, visible from the opponents :
    myCurrentHandContext->myCash = myCash;
    myCurrentHandContext->mySet = mySet;
    myCurrentHandContext->myM = static_cast<int>(getM());
    myCurrentHandContext->myCurrentHandActions = myCurrentHandActions;
    myCurrentHandContext->myHavePosition = Player::getHavePosition(myPosition, currentHand->getRunningPlayerList());
    myCurrentHandContext->myPreflopIsAggressor = isAgressor(GameStatePreflop);
    myCurrentHandContext->myFlopIsAggressor = isAgressor(GameStateFlop);
    myCurrentHandContext->myTurnIsAggressor = isAgressor(GameStateTurn);
    myCurrentHandContext->myRiverIsAggressor = isAgressor(GameStateRiver);
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
