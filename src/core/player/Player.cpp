// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Player.h"

#include <core/engine/CardUtilities.h>
#include <core/engine/HandEvaluator.h>

#include <core/engine/model/Ranges.h>
#include <core/interfaces/IHand.h>
#include <core/interfaces/persistence/IHandAuditStore.h>
#include <core/interfaces/persistence/IPlayersStatisticsStore.h>
#include <core/player/range/RangeParser.h>
#include <core/player/strategy/CurrentHandContext.h>
#include <core/services/GlobalServices.h>
#include "Helpers.h"

#include <sstream>

namespace pkt::core::player
{

using namespace std;

Player::Player(const GameEvents& events, int id, std::string name, int sC, bool aS, int mB)
    : myID(id), myName(name), myCash(sC), myButton(mB), myEvents(events)
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
}

const PlayerPosition Player::getPosition() const
{
    return myPosition;
}

void Player::setPosition()
{

    myPosition = UNKNOWN;

    const int dealerPlayerId = currentHand->getDealerPlayerId();
    const int nbPlayers = currentHand->getSeatsList()->size();

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

    if (myID == dealerPlayerId)
    {
        myPosition = BUTTON;
    }
    else
    {

        // get my relative position from the dealer
        PlayerListIterator itC;
        PlayerList players = currentHand->getSeatsList();

        int pos = 0;

        if (myID > dealerPlayerId)
        {

            bool dealerFound = false;
            for (itC = players->begin(); itC != players->end(); ++itC)
            {
                if ((*itC)->getId() == dealerPlayerId)
                {
                    dealerFound = true;
                }
                else if (dealerFound)
                {
                    pos++;
                }
                if ((*itC)->getId() == myID)
                {
                    break;
                }
            }
            myPosition = onDealerPositionPlus[pos][nbPlayers];
        }
        else
        {
            // myId < dealerPlayerId
            bool myPositionFound = false;
            for (itC = players->begin(); itC != players->end(); ++itC)
            {

                if ((*itC)->getId() == myID)
                {
                    myPositionFound = true;
                }
                else if (myPositionFound)
                {
                    pos++;
                }
                if ((*itC)->getId() == dealerPlayerId)
                {
                    break;
                }
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

void Player::addBetAmount(int theValue)
{
    myLastRelativeSet = theValue;
    myTotalBetAmount += theValue;
    myCash -= theValue;
}

void Player::setSetNull()
{
    myTotalBetAmount = 0;
    myLastRelativeSet = 0;
}
int Player::getTotalBetAmount() const
{
    return myTotalBetAmount;
}
int Player::getLastRelativeSet() const
{
    return myLastRelativeSet;
}

void Player::setAction(ActionType theValue, bool blind)
{
    myAction = theValue;
}
ActionType Player::getAction() const
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

void Player::setCards(int* theValue)
{

    for (int i = 0; i < 2; i++)
    {
        myCards[i] = theValue[i];
    }

    // will contain human-readable string, i.e "Qc" or "Ts"
    myCard1 = CardUtilities::getCardString(myCards[0]);
    myCard2 = CardUtilities::getCardString(myCards[1]);
}
void Player::getCards(int* theValue) const
{
    int i;
    for (i = 0; i < 2; i++)
    {
        theValue[i] = myCards[i];
    }
}

void Player::setCardsFlip(bool theValue)
{
    myCardsFlip = theValue;
}
bool Player::getCardsFlip() const
{
    return myCardsFlip;
}

void Player::setHandRanking(int theValue)
{
    myHandRanking = theValue;
}
int Player::getHandRanking() const
{
    return myHandRanking;
}

void Player::setCashAtHandStart(int theValue)
{
    myCashAtHandStart = theValue;
}
int Player::getCashAtHandStart() const
{
    return myCashAtHandStart;
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
    {
        myStatistics[i].reset();
    }
}

void Player::updatePreflopStatistics()
{

    const int nbPlayers = currentHand->getSeatsList()->size();

    if (myCurrentHandActions.getPreflopActions().size() == 1)
    {
        myStatistics[nbPlayers].totalHands++;
        myStatistics[nbPlayers].preflopStatistics.hands++;
    }

    switch (myAction)
    {
    case ActionType::Allin:
        myStatistics[nbPlayers].preflopStatistics.raises++;
        break;
    case ActionType::Raise:
        myStatistics[nbPlayers].preflopStatistics.raises++;
        break;
    case ActionType::Fold:
        myStatistics[nbPlayers].preflopStatistics.folds++;
        break;
    case ActionType::Check:
        myStatistics[nbPlayers].preflopStatistics.checks++;
        break;
    case ActionType::Call:
        myStatistics[nbPlayers].preflopStatistics.calls++;
        break;
    default:
        break;
    }

    myStatistics[nbPlayers].preflopStatistics.addLastAction(myAction); // keep track of the last 10 actions

    if (myAction == ActionType::Call && currentHand->getRaisersPositions().size() == 0)
    { //
        myStatistics[nbPlayers].preflopStatistics.limps++;
    }

    int playerRaises = 0;
    for (std::vector<ActionType>::const_iterator i = myCurrentHandActions.getPreflopActions().begin();
         i != myCurrentHandActions.getPreflopActions().end(); i++)
    {
        if (*i == ActionType::Raise || *i == ActionType::Allin)
        {
            playerRaises++;
        }
    }

    if (myAction == ActionType::Raise || myAction == ActionType::Allin)
    {

        if (playerRaises == 1 && currentHand->getRaisersPositions().size() == 2)
        {
            myStatistics[nbPlayers].preflopStatistics.threeBets++;
        }

        if (playerRaises == 2 || (playerRaises == 1 && currentHand->getRaisersPositions().size() == 3))
        {
            myStatistics[nbPlayers].preflopStatistics.fourBets++;
        }
    }
    else
    {

        if (playerRaises == 1)
        {

            myStatistics[nbPlayers].preflopStatistics.callthreeBetsOpportunities++;

            if (myAction == ActionType::Call)
            {
                myStatistics[nbPlayers].preflopStatistics.callthreeBets++;
            }
        }
    }
}
void Player::updateFlopStatistics()
{

    const int nbPlayers = currentHand->getSeatsList()->size();

    if (myCurrentHandActions.getFlopActions().size() == 1)
    {
        myStatistics[nbPlayers].flopStatistics.hands++;
    }

    switch (myAction)
    {
    case ActionType::Allin:
        myStatistics[nbPlayers].flopStatistics.raises++;
        break;
    case ActionType::Raise:
        myStatistics[nbPlayers].flopStatistics.raises++;
        break;
    case ActionType::Fold:
        myStatistics[nbPlayers].flopStatistics.folds++;
        break;
    case ActionType::Check:
        myStatistics[nbPlayers].flopStatistics.checks++;
        break;
    case ActionType::Call:
        myStatistics[nbPlayers].flopStatistics.calls++;
        break;
    case ActionType::Bet:
        myStatistics[nbPlayers].flopStatistics.bets++;
        break;
    default:
        break;
    }
    if (myAction == ActionType::Raise && currentHand->getRaisersPositions().size() > 1)
    {
        myStatistics[nbPlayers].flopStatistics.threeBets++;
    }

    // continuation bets
    if (currentHand->getPreflopLastRaiserId() == myID)
    {
        myStatistics[nbPlayers].flopStatistics.continuationBetsOpportunities++;
        if (myAction == ActionType::Bet)
        {
            myStatistics[nbPlayers].flopStatistics.continuationBets++;
        }
    }
}
void Player::updateTurnStatistics()
{

    const int nbPlayers = currentHand->getSeatsList()->size();

    if (myCurrentHandActions.getTurnActions().size() == 1)
    {
        myStatistics[nbPlayers].turnStatistics.hands++;
    }

    switch (myAction)
    {
    case ActionType::Allin:
        myStatistics[nbPlayers].turnStatistics.raises++;
        break;
    case ActionType::Raise:
        myStatistics[nbPlayers].turnStatistics.raises++;
        break;
    case ActionType::Fold:
        myStatistics[nbPlayers].turnStatistics.folds++;
        break;
    case ActionType::Check:
        myStatistics[nbPlayers].turnStatistics.checks++;
        break;
    case ActionType::Call:
        myStatistics[nbPlayers].turnStatistics.calls++;
        break;
    case ActionType::Bet:
        myStatistics[nbPlayers].turnStatistics.bets++;
        break;
    default:
        break;
    }
    if (myAction == ActionType::Raise && currentHand->getRaisersPositions().size() > 1)
    {
        myStatistics[nbPlayers].turnStatistics.threeBets++;
    }
}
void Player::updateRiverStatistics()
{

    const int nbPlayers = currentHand->getSeatsList()->size();

    if (myCurrentHandActions.getRiverActions().size() == 1)
    {
        myStatistics[nbPlayers].riverStatistics.hands++;
    }

    switch (myAction)
    {
    case ActionType::Allin:
        myStatistics[nbPlayers].riverStatistics.raises++;
        break;
    case ActionType::Raise:
        myStatistics[nbPlayers].riverStatistics.raises++;
        break;
    case ActionType::Fold:
        myStatistics[nbPlayers].riverStatistics.folds++;
        break;
    case ActionType::Check:
        myStatistics[nbPlayers].riverStatistics.checks++;
        break;
    case ActionType::Call:
        myStatistics[nbPlayers].riverStatistics.calls++;
        break;
    case ActionType::Bet:
        myStatistics[nbPlayers].riverStatistics.bets++;
        break;
    default:
        break;
    }
    if (myAction == ActionType::Raise && currentHand->getRaisersPositions().size() > 1)
    {
        myStatistics[nbPlayers].riverStatistics.threeBets++;
    }
}

void Player::loadStatistics()
{

    resetPlayerStatistics(); // reset stats to 0

    myStatistics = GlobalServices::instance().playersStatisticsStore()->getPlayerStatistics(myName);
    if (myStatistics.empty())
    {
        myStatistics.fill(PlayerStatistics());
    }
}

const PostFlopAnalysisFlags Player::getPostFlopAnalysisFlags() const
{
    std::string stringHand = getCardsValueString();
    std::string stringBoard = getStringBoard();

    return GlobalServices::instance().handEvaluationEngine()->analyzeHand(getCardsValueString(), getStringBoard());
}
bool Player::checkIfINeedToShowCards() const
{
    std::list<unsigned> playerNeedToShowCardsList = currentHand->getBoard()->getPlayerNeedToShowCards();
    for (std::list<unsigned>::iterator it = playerNeedToShowCardsList.begin(); it != playerNeedToShowCardsList.end();
         ++it)
    {
        if (*it == myID)
        {
            return true;
        }
    }

    return false;
}
std::string Player::getStringBoard() const
{

    int cardsOnBoard;

    if (currentHand->getCurrentRoundState() == Flop)
    {
        cardsOnBoard = 3;
    }
    else if (currentHand->getCurrentRoundState() == Turn)
    {
        cardsOnBoard = 4;
    }
    else if (currentHand->getCurrentRoundState() == River)
    {
        cardsOnBoard = 5;
    }
    else
    {
        cardsOnBoard = 0;
    }

    std::string stringBoard;
    int board[5];
    currentHand->getBoard()->getCards(board);

    for (int i = 0; i < cardsOnBoard; i++)
    {
        stringBoard += " ";
        stringBoard += CardUtilities::getCardString(board[i]);
    }

    return stringBoard;
}

bool Player::hasPosition(PlayerPosition myPos, PlayerList runningPlayers)
{
    // return true if myPos is last to play, false if not

    bool hasPosition = true;

    PlayerListConstIterator itC;

    for (itC = runningPlayers->begin(); itC != runningPlayers->end(); ++itC)
    {

        if ((*itC)->getPosition() > myPos)
        {
            hasPosition = false;
        }
    }

    return hasPosition;
}

int Player::getPotOdd() const
{

    const int highestBetAmount = min(myCash, currentHand->getCurrentBettingRound()->getHighestSet());

    int pot = currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets();

    if (pot == 0)
    { // shouldn't happen, but...
        GlobalServices::instance().logger()->error("Pot = " + std::to_string(currentHand->getBoard()->getPot()) +
                                                   " + " + std::to_string(currentHand->getBoard()->getSets()) + " = " +
                                                   std::to_string(pot));
        return 0;
    }

    int odd = (highestBetAmount - myTotalBetAmount) * 100 / pot;
    if (odd < 0)
    {
        odd = -odd; // happens if myTotalBetAmount > highestBetAmount
    }

    return odd;
}

float Player::getM() const
{

    int blinds = currentHand->getSmallBlind() + (currentHand->getSmallBlind() * 2); // assume for now that BB is 2 * SB
    if (blinds > 0 && myCash > 0)
    {
        return (float) myCash / blinds;
    }
    else
    {
        return 0;
    }
}

const HandSimulationStats Player::computeHandSimulation() const
{
    const int nbOpponents = currentHand->getSeatsList()->size() - 1;
    // evaluate my strength against my opponents's guessed ranges :
    float maxOpponentsStrengths = getMaxOpponentsStrengths();
    return GlobalServices::instance().handEvaluationEngine()->simulateHandEquity(
        getCardsValueString(), getStringBoard(), nbOpponents, maxOpponentsStrengths);
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
        {
            maxOpponentsStrengths = i->second;
        }

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

    auto opponent = *getPlayerListIteratorById(currentHand->getSeatsList(), opponentId);

    assert(myHandRanking > 0);

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

        // delete hands that can't exist, given the board
        if (board.find(s1) != string::npos || board.find(s2) != string::npos)
        {
            continue;
        }

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
        const int rank = GlobalServices::instance().handEvaluationEngine()->rankHand(((*i) + board).c_str());
        if (rank > myHandRanking)
        {
            nbWinningHands++;
        }
    }
    if (ranges.size() == 0)
    {
        GlobalServices::instance().logger()->error("no ranges for opponent " + std::to_string(opponentId));
        return 0;
    }
    assert(nbWinningHands / ranges.size() <= 1.0);
    return (float) nbWinningHands / (float) newRanges.size();
}

std::map<int, float> Player::evaluateOpponentsStrengths() const
{

    map<int, float> result;
    PlayerList players = currentHand->getSeatsList();
    const int nbPlayers = currentHand->getSeatsList()->size();

    for (PlayerListIterator it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getId() == myID || (*it)->getAction() == ActionType::Fold || (*it)->getAction() == ActionType::None)
        {
            continue;
        }

        const float estimatedOpponentWinningHands = getOpponentWinningHandsPercentage((*it)->getId(), getStringBoard());
        assert(estimatedOpponentWinningHands <= 1.0);

        result[(*it)->getId()] = estimatedOpponentWinningHands;
    }

    return result;
}

bool Player::isPreflopBigBet() const
{

    if (getPotOdd() > 70)
    {
        return true;
    }

    const int highestBetAmount = min(myCash, currentHand->getCurrentBettingRound()->getHighestSet());

    if (highestBetAmount > currentHand->getSmallBlind() * 8 &&
        highestBetAmount - myTotalBetAmount > myTotalBetAmount * 6)
    {
        return true;
    }

    return false;
}

bool Player::isAgressor(const GameState gameState) const
{

    if (gameState == Preflop && currentHand->getPreflopLastRaiserId() == myID)
    {
        return true;
    }

    if (gameState == Flop && currentHand->getFlopLastRaiserId() == myID)
    {
        return true;
    }

    if (gameState == Turn && currentHand->getTurnLastRaiserId() == myID)
    {
        return true;
    }

    if (gameState == River && currentHand->getLastRaiserId() == myID)
    {
        return true;
    }

    return false;
}
bool Player::canBluff(const GameState gameState) const
{

    // check if there is no calling station at the table
    // check also if my opponents stacks are big enough to bluff them

    const int nbPlayers = currentHand->getSeatsList()->size();
    const int nbRaises = currentHand->getPreflopRaisesNumber();

    PlayerList players = currentHand->getRunningPlayersList();

    if (players->size() == 1)
    {
        // all other players are allin
        return false;
    }

    for (PlayerListIterator it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getId() == myID)
        {
            continue;
        }

        PreflopStatistics preflopStats = (*it)->getStatistics(nbPlayers).preflopStatistics;

        // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more
        // accurate
        if (preflopStats.hands < MIN_HANDS_STATISTICS_ACCURATE && nbPlayers < 10 &&
            (*it)->getStatistics(nbPlayers + 1).preflopStatistics.hands > MIN_HANDS_STATISTICS_ACCURATE)
        {

            preflopStats = (*it)->getStatistics(nbPlayers + 1).preflopStatistics;
        }

        if ((*it)->getStatistics(nbPlayers).getWentToShowDown() >= 40 &&
            preflopStats.getVoluntaryPutMoneyInPot() - preflopStats.getPreflopRaise() > 15 &&
            preflopStats.getVoluntaryPutMoneyInPot() > 20)
        {
            return false; // seems to be a calling station
        }

        if ((*it)->getCash() < currentHand->getBoard()->getPot() * 3)
        {
            return false;
        }

        if (gameState == Preflop)
        {
            if (preflopStats.getPreflopCallthreeBetsFrequency() > 40)
            {
                return false;
            }
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

    PreflopStatistics preflop = stats.preflopStatistics;

    if (preflop.getLastActionsNumber(ActionType::Allin) + preflop.getLastActionsNumber(ActionType::Raise) +
            preflop.getLastActionsNumber(ActionType::Call) >
        PreflopStatistics::LAST_ACTIONS_STACK_SIZE * 0.8)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Player::updateCurrentHandContext(const GameState state)
{
    // general (and shared) game state
    myCurrentHandContext->commonContext.gameState = state;
    myCurrentHandContext->commonContext.nbRunningPlayers = currentHand->getRunningPlayersList()->size();
    myCurrentHandContext->commonContext.lastVPIPPlayer =
        getPlayerById(currentHand->getSeatsList(), currentHand->getLastRaiserId());
    myCurrentHandContext->commonContext.callersPositions = currentHand->getCallersPositions();
    myCurrentHandContext->commonContext.pot = currentHand->getBoard()->getPot();
    myCurrentHandContext->commonContext.potOdd = getPotOdd();
    myCurrentHandContext->commonContext.sets = currentHand->getBoard()->getSets();
    myCurrentHandContext->commonContext.highestBetAmount = currentHand->getCurrentBettingRound()->getHighestSet();
    myCurrentHandContext->commonContext.stringBoard = getStringBoard();
    myCurrentHandContext->commonContext.preflopLastRaiser =
        getPlayerById(currentHand->getSeatsList(), currentHand->getPreflopLastRaiserId());
    myCurrentHandContext->commonContext.preflopRaisesNumber = currentHand->getPreflopRaisesNumber();
    myCurrentHandContext->commonContext.preflopCallsNumber = currentHand->getPreflopCallsNumber();
    myCurrentHandContext->commonContext.isPreflopBigBet = isPreflopBigBet();
    myCurrentHandContext->commonContext.flopBetsOrRaisesNumber = currentHand->getFlopBetsOrRaisesNumber();
    myCurrentHandContext->commonContext.flopLastRaiser =
        getPlayerById(currentHand->getSeatsList(), currentHand->getFlopLastRaiserId());
    myCurrentHandContext->commonContext.turnBetsOrRaisesNumber = currentHand->getTurnBetsOrRaisesNumber();
    myCurrentHandContext->commonContext.turnLastRaiser =
        getPlayerById(currentHand->getSeatsList(), currentHand->getTurnLastRaiserId());
    myCurrentHandContext->commonContext.riverBetsOrRaisesNumber = currentHand->getRiverBetsOrRaisesNumber();
    myCurrentHandContext->commonContext.nbPlayers = currentHand->getSeatsList()->size();
    myCurrentHandContext->commonContext.smallBlind = currentHand->getSmallBlind();

    for (std::vector<ActionType>::const_iterator i = myCurrentHandActions.getFlopActions().begin();
         i != myCurrentHandActions.getFlopActions().end(); i++)
    {

        if (*i == ActionType::Raise)
        {
            myCurrentHandContext->perPlayerContext.nbRaises++;
        }
        else if (*i == ActionType::Bet)
        {
            myCurrentHandContext->perPlayerContext.nbBets++;
        }
        else if (*i == ActionType::Check)
        {
            myCurrentHandContext->perPlayerContext.nbChecks++;
        }
        else if (*i == ActionType::Call)
        {
            myCurrentHandContext->perPlayerContext.nbCalls++;
        }
        else if (*i == ActionType::Allin)
        {
            myCurrentHandContext->perPlayerContext.nbAllins++;
        }
    }

    // Player-specific, visible from the opponents :
    myCurrentHandContext->perPlayerContext.myCash = myCash;
    myCurrentHandContext->perPlayerContext.myTotalBetAmount = myTotalBetAmount;
    myCurrentHandContext->perPlayerContext.myM = static_cast<int>(getM());
    myCurrentHandContext->perPlayerContext.myCurrentHandActions = myCurrentHandActions;
    myCurrentHandContext->perPlayerContext.myHavePosition =
        Player::hasPosition(myPosition, currentHand->getRunningPlayersList());
    myCurrentHandContext->perPlayerContext.myPreflopIsAggressor = isAgressor(Preflop);
    myCurrentHandContext->perPlayerContext.myFlopIsAggressor = isAgressor(Flop);
    myCurrentHandContext->perPlayerContext.myTurnIsAggressor = isAgressor(Turn);
    myCurrentHandContext->perPlayerContext.myRiverIsAggressor = isAgressor(River);
    myCurrentHandContext->perPlayerContext.myStatistics = getStatistics(currentHand->getSeatsList()->size());
    myCurrentHandContext->perPlayerContext.myID = myID;
    myCurrentHandContext->perPlayerContext.myIsInVeryLooseMode = isInVeryLooseMode(currentHand->getSeatsList()->size());
    myCurrentHandContext->perPlayerContext.myPosition = myPosition;

    // player specific, hidden from the opponents :
    myCurrentHandContext->perPlayerContext.myCard1 = myCard1;
    myCurrentHandContext->perPlayerContext.myCard2 = myCard2;
    myCurrentHandContext->perPlayerContext.myCanBluff = canBluff(state);
    myCurrentHandContext->perPlayerContext.myPreflopCallingRange = calculatePreflopCallingRange(*myCurrentHandContext);
    // guess what the opponents might have and evaluate our strength, given our hole cards and the board cards (if any)
    myCurrentHandContext->perPlayerContext.myHandSimulation = computeHandSimulation();
    myCurrentHandContext->perPlayerContext.myPostFlopAnalysisFlags = getPostFlopAnalysisFlags();
}

float Player::calculatePreflopCallingRange(const CurrentHandContext& ctx) const
{
    return myRangeEstimator->getStandardCallingRange(currentHand->getSeatsList()->size());
}

} // namespace pkt::core::player
