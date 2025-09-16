// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Player.h"

#include <core/engine/CardUtilities.h>
#include <core/engine/Hand.h>
#include <core/engine/HandEvaluator.h>
#include <core/engine/Helpers.h>
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

Player::Player(const GameEvents& events, int id, std::string name, int cash)
    : myID(id), myName(name), myCash(cash), myEvents(events)
{
    myRangeEstimator = std::make_unique<RangeEstimator>(myID);
    myCurrentHandContext = std::make_unique<CurrentHandContext>();
    myStatisticsUpdater = std::make_unique<PlayerStatisticsUpdater>();
    myStatisticsUpdater->loadStatistics(name);

    // Initialize with invalid cards
    myHoleCards.reset();
}

const PlayerPosition Player::getPosition() const
{
    return myPosition;
}

int Player::getId() const
{
    return myID;
}
const CurrentHandActions& Player::getCurrentHandActions() const
{
    return myCurrentHandActions;
}

std::string Player::getName() const
{
    return myName;
}

void Player::setCash(int theValue)
{
    myCash = theValue;
    if (myEvents.onPlayerChipsUpdated)
    {
        myEvents.onPlayerChipsUpdated(myID, myCash);
    }
}
int Player::getCash() const
{
    return myCash;
}

void Player::addBetAmount(int theValue)
{
    myCash -= theValue;
    if (myEvents.onPlayerChipsUpdated)
    {
        myEvents.onPlayerChipsUpdated(myID, myCash);
    }
}

PlayerAction Player::getLastAction() const
{
    return getCurrentHandActions().getLastAction();
}

// will set hole cards from int array, maintaining backward compatibility
void Player::setCards(int* theValue)
{
    myHoleCards.fromIntArray(theValue);
}

void Player::getCards(int* theValue) const
{
    myHoleCards.toIntArray(theValue);
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
    return myHoleCards.toString();
}

const HandSimulationStats Player::computeHandSimulation() const
{
#if (False)
    const int nbOpponents = mySeatsList->size() - 1;
    // evaluate my strength against my opponents's guessed ranges :
    float maxOpponentsStrengths = getMaxOpponentsStrengths();
    return GlobalServices::instance().handEvaluationEngine().simulateHandEquity(getCardsValueString(), getStringBoard(),
                                                                                nbOpponents, maxOpponentsStrengths);
#else
    return GlobalServices::instance().handEvaluationEngine().simulateHandEquity("As 6d", "", 2, 0.5);
#endif
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
float Player::getOpponentWinningHandsPercentage(const int opponentId, std::string board) const
{
#if (false)
    float result = 0;

    auto opponent = *getPlayerListIteratorById(myCurrentHandContext->commonContext.mySeatsList, opponentId);

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
            GlobalServices::instance().logger().error("invalid hand : " + (*i));
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
            const std::string card1Str = myHoleCards.card1.toString();
            const std::string card2Str = myHoleCards.card2.toString();
            if (card1Str.find(s1) != string::npos || card2Str.find(s1) != string::npos ||
                card1Str.find(s2) != string::npos || card2Str.find(s2) != string::npos)
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
        const int rank = GlobalServices::instance().handEvaluationEngine().rankHand(((*i) + board).c_str());
        if (rank > myHandRanking)
        {
            nbWinningHands++;
        }
    }
    if (ranges.size() == 0)
    {
        GlobalServices::instance().logger().error("no ranges for opponent " + std::to_string(opponentId));
        return 0;
    }
    assert(nbWinningHands / ranges.size() <= 1.0);
    return (float) nbWinningHands / (float) newRanges.size();
#else
    return 0;
#endif
}

std::map<int, float> Player::evaluateOpponentsStrengths() const
{

    map<int, float> result;
#if (False)
    PlayerList players = mySeatsList;
    const int nbPlayers = mySeatsList->size();

    for (PlayerListIterator it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getId() == myID || (*it)->getLastAction().type == ActionType::Fold ||
            (*it)->getLastAction().type == ActionType::None)
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

bool Player::isPreflopBigBet() const
{
#if (False)
    if (getPotOdd() > 70)
    {
        return true;
    }

    const int highestBetAmount = min(myCash, currentHand.getCurrentBettingRound()->getRoundHighestSet());

    if (highestBetAmount > currentHand.getSmallBlind() * 8 &&
        highestBetAmount - myTotalBetAmount > myTotalBetAmount * 6)
    {
        return true;
    }
#endif
    return false;
}

bool Player::isAgressor(const GameState gameState) const
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

void Player::updateCurrentHandContext(const GameState state, Hand& currentHand)
{
    // common context
    myCurrentHandContext->commonContext = currentHand.updateHandCommonContext(state);

    // Calculate valid actions for this specific player
    myCurrentHandContext->commonContext.validActions = pkt::core::getValidActionsForPlayer(currentHand, myID);

    // Player-specific, visible from the opponents :
    myCurrentHandContext->personalContext.cash = myCash;
    myCurrentHandContext->personalContext.totalBetAmount = myCurrentHandActions.getHandTotalBetAmount();
    myCurrentHandContext->personalContext.m = static_cast<int>(currentHand.getM(myCash));
    myCurrentHandContext->personalContext.actions.currentHandActions = myCurrentHandActions;
    myCurrentHandContext->personalContext.hasPosition = hasPosition(myPosition, currentHand.getActingPlayersList());
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
    myCurrentHandContext->personalContext.holeCards = myHoleCards;
    myCurrentHandContext->personalContext.preflopCallingRange = calculatePreflopCallingRange(*myCurrentHandContext);
    // guess what the opponents might have and evaluate our strength, given our hole cards and the board cards (if any)
    myCurrentHandContext->personalContext.myHandSimulation = computeHandSimulation();
    myCurrentHandContext->personalContext.postFlopAnalysisFlags = getPostFlopAnalysisFlags();
}

float Player::calculatePreflopCallingRange(const CurrentHandContext& ctx) const
{
    return myRangeEstimator->getStandardCallingRange(ctx.commonContext.playersContext.actingPlayersList->size());
}

void Player::resetForNewHand(const Hand& hand)
{
    setCardsFlip(0);
    myCurrentHandActions.reset();
    setCashAtHandStart(getCash());
    setPosition(hand);
    getRangeEstimator()->setEstimatedRange("");
}
const PostFlopAnalysisFlags Player::getPostFlopAnalysisFlags() const
{
    std::string stringHand = getCardsValueString();
    std::string stringBoard = myCurrentHandContext->commonContext.stringBoard;

    return GlobalServices::instance().handEvaluationEngine().analyzeHand(getCardsValueString(), stringBoard);
}

bool Player::checkIfINeedToShowCards() const
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
void Player::setPosition(const Hand& hand)
{
    myPosition = Unknown;

    const int dealerId = hand.getDealerPlayerId();
    const PlayerList players = hand.getSeatsList();
    const int nbPlayers = players->size();

    // Compute relative offset clockwise from dealer
    int offset = playerDistanceCircularOffset(dealerId, myID, players);
    myPosition = computePlayerPositionFromOffset(offset, nbPlayers);

    assert(myPosition != Unknown);
}

void Player::setAction(IHandState& state, const PlayerAction& action)
{
    if (action.type != ActionType::None)
    {
        GlobalServices::instance().logger().info(myName + " " + std::string(actionTypeToString(action.type)) +
                                                 (action.amount ? " " + std::to_string(action.amount) : ""));
    }
    myCurrentHandActions.addAction(state.getGameState(), action);
}

const PlayerStatistics& Player::getStatistics(const int nbPlayers) const
{
    return myStatisticsUpdater->getStatistics(nbPlayers);
}
} // namespace pkt::core::player
