// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "PlayerFsm.h"

#include <core/engine/CardUtilities.h>
#include <core/engine/HandEvaluator.h>

#include <core/engine/HandFsm.h>
#include <core/engine/model/Ranges.h>
#include <core/interfaces/persistence/PlayersStatisticsStore.h>
#include <core/player/range/RangeParser.h>
#include <core/player/strategy/CurrentHandContext.h>
#include <core/services/GlobalServices.h>
#include "Helpers.h"

#include <sstream>

namespace pkt::core::player
{

using namespace std;

PlayerFsm::PlayerFsm(const GameEvents& events, int id, std::string name, int cash)
    : myID(id), myName(name), myCash(cash), myEvents(events)
{
    myRangeEstimator = std::make_unique<RangeEstimator>(myID);
    myCurrentHandContext = std::make_unique<CurrentHandContext>();
    myStatisticsUpdater = std::make_unique<PlayerStatisticsUpdater>();
    myStatisticsUpdater->loadStatistics(name);

    int i;
    for (i = 0; i < 2; i++)
    {
        myCards[i] = -1;
    }
}

const PlayerPosition PlayerFsm::getPosition() const
{
    return myPosition;
}

int PlayerFsm::getId() const
{
    return myID;
}
CurrentHandActions& PlayerFsm::getCurrentHandActions()
{
    return myCurrentHandActions;
}

std::string PlayerFsm::getName() const
{
    return myName;
}

void PlayerFsm::setCash(int theValue)
{
    myCash = theValue;
}
int PlayerFsm::getCash() const
{
    return myCash;
}

void PlayerFsm::addBetAmount(int theValue)
{
    myLastBetAmount = theValue;
    myTotalBetAmount += theValue;
    myCash -= theValue;
}

void PlayerFsm::resetBetAmount()
{
    myTotalBetAmount = 0;
    myLastBetAmount = 0;
}
int PlayerFsm::getTotalBetAmount() const
{
    return myTotalBetAmount;
}
void PlayerFsm::setTotalBetAmount(int theValue)
{
    myTotalBetAmount = theValue;
}
int PlayerFsm::getLastBetAmount() const
{
    return myLastBetAmount;
}

void PlayerFsm::setAction(ActionType theValue, bool blind)
{
    myAction = theValue;
}
ActionType PlayerFsm::getAction() const
{
    return myAction;
}

// will contain human-readable string, i.e "Qc" or "Ts"
void PlayerFsm::setCards(int* theValue)
{

    for (int i = 0; i < 2; i++)
    {
        myCards[i] = theValue[i];
    }

    myCard1 = CardUtilities::getCardString(myCards[0]);
    myCard2 = CardUtilities::getCardString(myCards[1]);
}
void PlayerFsm::getCards(int* theValue) const
{
    int i;
    for (i = 0; i < 2; i++)
    {
        theValue[i] = myCards[i];
    }
}

void PlayerFsm::setCardsFlip(bool theValue)
{
    myCardsFlip = theValue;
}
bool PlayerFsm::getCardsFlip() const
{
    return myCardsFlip;
}

void PlayerFsm::setHandRanking(int theValue)
{
    myHandRanking = theValue;
}
int PlayerFsm::getHandRanking() const
{
    return myHandRanking;
}

void PlayerFsm::setCashAtHandStart(int theValue)
{
    myCashAtHandStart = theValue;
}
int PlayerFsm::getCashAtHandStart() const
{
    return myCashAtHandStart;
}

void PlayerFsm::setLastMoneyWon(int theValue)
{
    lastMoneyWon = theValue;
}
int PlayerFsm::getLastMoneyWon() const
{
    return lastMoneyWon;
}

std::string PlayerFsm::getCardsValueString() const
{
    std::string s = myCard1 + " " + myCard2;
    return s;
}

const HandSimulationStats PlayerFsm::computeHandSimulation() const
{
#if (False)
    const int nbOpponents = mySeatsList->size() - 1;
    // evaluate my strength against my opponents's guessed ranges :
    float maxOpponentsStrengths = getMaxOpponentsStrengths();
    return GlobalServices::instance().handEvaluationEngine()->simulateHandEquity(
        getCardsValueString(), getStringBoard(), nbOpponents, maxOpponentsStrengths);
#else
    return GlobalServices::instance().handEvaluationEngine()->simulateHandEquity("As 6d", "", 2, 0.5);
#endif
}

float PlayerFsm::getMaxOpponentsStrengths() const
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
    //	const int nbRaises =	currentHand.getFlopBetsOrRaisesNumber() +
    //							currentHand.getTurnBetsOrRaisesNumber() +
    //							currentHand.getRiverBetsOrRaisesNumber();
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
float PlayerFsm::getOpponentWinningHandsPercentage(const int opponentId, std::string board) const
{
#if (false)
    float result = 0;

    auto opponent = *getPlayerFsmListIteratorById(myCurrentHandContext->commonContext.mySeatsList, opponentId);

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
#else
    return 0;
#endif
}

std::map<int, float> PlayerFsm::evaluateOpponentsStrengths() const
{

    map<int, float> result;
#if (False)
    PlayerFsmList players = mySeatsList;
    const int nbPlayers = mySeatsList->size();

    for (PlayerFsmListIterator it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getId() == myID || (*it)->getAction() == ActionType::Fold || (*it)->getAction() == ActionType::None)
        {
            continue;
        }

        const float estimatedOpponentWinningHands = getOpponentWinningHandsPercentage((*it)->getId(), getStringBoard());
        assert(estimatedOpponentWinningHands <= 1.0);

        result[(*it)->getId()] = estimatedOpponentWinningHands;
    }
#endif

    return result;
}

bool PlayerFsm::isPreflopBigBet() const
{
#if (False)
    if (getPotOdd() > 70)
    {
        return true;
    }

    const int highestBetAmount = min(myCash, currentHand.getCurrentBettingRound()->getHighestSet());

    if (highestBetAmount > currentHand.getSmallBlind() * 8 &&
        highestBetAmount - myTotalBetAmount > myTotalBetAmount * 6)
    {
        return true;
    }
#endif
    return false;
}

bool PlayerFsm::isAgressor(const GameState gameState) const
{
#if (False)
    if (gameState == Preflop && currentHand.getPreflopLastRaiserId() == myID)
    {
        return true;
    }

    if (gameState == Flop && currentHand.getFlopLastRaiserId() == myID)
    {
        return true;
    }

    if (gameState == Turn && currentHand.getTurnLastRaiserId() == myID)
    {
        return true;
    }

    if (gameState == River && currentHand.getLastRaiserId() == myID)
    {
        return true;
    }
#endif
    return false;
}

int PlayerFsm::getPreflopPotOdd() const
{
    return myPreflopPotOdd;
}

void PlayerFsm::setPreflopPotOdd(const int potOdd)
{
    myPreflopPotOdd = potOdd;
}

bool PlayerFsm::isInVeryLooseMode(const int nbPlayers) const
{
    PlayerStatistics stats = myStatisticsUpdater->getStatistics(nbPlayers);

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

void PlayerFsm::updateCurrentHandContext(const GameState state, HandFsm& currentHand)
{
    // common context
    myCurrentHandContext->commonContext = currentHand.updateHandCommonContext(state);

    // Player-specific, visible from the opponents :
    myCurrentHandContext->personalContext.cash = myCash;
    myCurrentHandContext->personalContext.totalBetAmount = myTotalBetAmount;
    myCurrentHandContext->personalContext.m = static_cast<int>(currentHand.getM(myCash));
    myCurrentHandContext->personalContext.actions.currentHandActions = myCurrentHandActions;
    myCurrentHandContext->personalContext.hasPosition = hasPosition(myPosition, currentHand.getRunningPlayersList());
    myCurrentHandContext->personalContext.actions.preflopIsAggressor = isAgressor(Preflop);
    myCurrentHandContext->personalContext.actions.flopIsAggressor = isAgressor(Flop);
    myCurrentHandContext->personalContext.actions.turnIsAggressor = isAgressor(Turn);
    myCurrentHandContext->personalContext.actions.riverIsAggressor = isAgressor(River);
    myCurrentHandContext->personalContext.statistics =
        myStatisticsUpdater->getStatistics(myCurrentHandContext->commonContext.playersContext.nbPlayers);
    myCurrentHandContext->personalContext.id = myID;
    myCurrentHandContext->personalContext.actions.isInVeryLooseMode =
        isInVeryLooseMode(myCurrentHandContext->commonContext.playersContext.nbPlayers);
    myCurrentHandContext->personalContext.position = myPosition;

    // player specific, hidden from the opponents :
    myCurrentHandContext->personalContext.card1 = myCard1;
    myCurrentHandContext->personalContext.card2 = myCard2;
    myCurrentHandContext->personalContext.preflopCallingRange = calculatePreflopCallingRange(*myCurrentHandContext);
    // guess what the opponents might have and evaluate our strength, given our hole cards and the board cards (if any)
    myCurrentHandContext->personalContext.myHandSimulation = computeHandSimulation();
    myCurrentHandContext->personalContext.postFlopAnalysisFlags = getPostFlopAnalysisFlags();
}

float PlayerFsm::calculatePreflopCallingRange(const CurrentHandContext& ctx) const
{
    return myRangeEstimator->getStandardCallingRange(ctx.commonContext.playersContext.runningPlayersListFsm->size());
}

void PlayerFsm::resetForNewHand(const HandFsm& hand)
{
    setCardsFlip(0);
    getCurrentHandActions().reset();
    setCashAtHandStart(getCash());
    setPosition(hand);
    getRangeEstimator()->setEstimatedRange("");
}
const PostFlopAnalysisFlags PlayerFsm::getPostFlopAnalysisFlags() const
{
    std::string stringHand = getCardsValueString();
    std::string stringBoard = myCurrentHandContext->commonContext.stringBoard;

    return GlobalServices::instance().handEvaluationEngine()->analyzeHand(getCardsValueString(), stringBoard);
}

bool PlayerFsm::checkIfINeedToShowCards() const
{
#if false
    std::list<unsigned> playerNeedToShowCardsList = currentHand.getBoard()->getPlayerNeedToShowCards();
    for (std::list<unsigned>::iterator it = playerNeedToShowCardsList.begin(); it != playerNeedToShowCardsList.end();
         ++it)
    {
        if (*it == myID)
        {
            return true;
        }
    }
#endif
    return false;
}
void PlayerFsm::setPosition(const HandFsm& hand)
{
    myPosition = Unknown;

    const int dealerId = hand.getDealerPlayerId();
    const PlayerFsmList players = hand.getSeatsList();
    const int nbPlayers = players->size();

    // Compute relative offset clockwise from dealer
    int offset = circularOffset(dealerId, myID, players);
    myPosition = computePositionFromOffset(offset, nbPlayers);

    assert(myPosition != Unknown);
}

} // namespace pkt::core::player
