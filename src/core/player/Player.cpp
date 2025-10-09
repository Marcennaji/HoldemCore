// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Player.h"

#include "core/interfaces/Logger.h"
#include <core/engine/cards/CardUtilities.h>
#include <core/engine/hand/Hand.h>
#include <core/engine/hand/HandEvaluator.h>
#include <core/engine/model/Ranges.h>
#include <core/engine/utils/Helpers.h>
#include <core/interfaces/persistence/PlayersStatisticsStore.h>
#include <core/player/range/RangeParser.h>
#include <core/player/strategy/CurrentHandContext.h>
#include <core/player/strategy/PreflopRangeCalculator.h>
#include "Helpers.h"

#include <sstream>

namespace pkt::core::player
{

using namespace std;


// Fully ISP-compliant constructor with all required interfaces
Player::Player(const GameEvents& events, pkt::core::Logger& logger, 
               pkt::core::HandEvaluationEngine& handEvaluator,
               pkt::core::PlayersStatisticsStore& statisticsStore,
               pkt::core::Randomizer& randomizer,
               int id, std::string name, int cash)
    : m_id(id), m_name(name), m_events(events), m_logger(&logger), m_handEvaluator(&handEvaluator), m_statisticsStore(&statisticsStore), m_randomizer(&randomizer)
{
    m_rangeEstimator = std::make_unique<RangeEstimator>(m_id, logger, handEvaluator);
    m_currentHandContext = std::make_unique<CurrentHandContext>();
    m_statisticsUpdater = std::make_unique<PlayerStatisticsUpdater>(statisticsStore);
    m_statisticsUpdater->loadStatistics(name);

    m_currentHandContext->personalContext.cash = cash;
}

const PlayerPosition Player::getPosition() const
{
    if (m_currentHandContext)
    {
        return m_currentHandContext->personalContext.position;
    }
    return PlayerPosition::Unknown; // Default when no context
}

int Player::getId() const
{
    return m_id;
}
const CurrentHandActions& Player::getCurrentHandActions() const
{
    return m_currentHandContext->personalContext.actions.currentHandActions;
}

std::string Player::getName() const
{
    return m_name;
}

void Player::setCash(int theValue)
{
    // Update context if it exists
    if (m_currentHandContext)
    {
        m_currentHandContext->personalContext.cash = theValue;
    }
    if (m_events.onPlayerChipsUpdated)
    {
        m_events.onPlayerChipsUpdated(m_id, theValue);
    }
}
int Player::getCash() const
{
    if (m_currentHandContext)
    {
        return m_currentHandContext->personalContext.cash;
    }
    return 0; // Default when no context
}

void Player::addBetAmount(int theValue)
{
    // Update context directly
    if (m_currentHandContext)
    {
        m_currentHandContext->personalContext.cash -= theValue;
    }
    if (m_events.onPlayerChipsUpdated)
    {
        m_events.onPlayerChipsUpdated(m_id, getCash());
    }
}

PlayerAction Player::getLastAction() const
{
    return getCurrentHandActions().getLastAction();
}

void Player::resetCurrentHandActions()
{
    m_currentHandContext->personalContext.actions.currentHandActions.reset();
}

const HoleCards& Player::getHoleCards() const
{
    return m_currentHandContext->personalContext.holeCards;
}

void Player::setHandRanking(int theValue)
{
    if (m_currentHandContext)
    {
        m_currentHandContext->personalContext.handRanking = theValue;
    }
}
int Player::getHandRanking() const
{
    if (m_currentHandContext)
    {
        return m_currentHandContext->personalContext.handRanking;
    }
    return 0; // Default when no context
}

void Player::setCashAtHandStart(int theValue)
{
    if (m_currentHandContext)
    {
        m_currentHandContext->personalContext.cashAtHandStart = theValue;
    }
}
int Player::getCashAtHandStart() const
{
    if (m_currentHandContext)
    {
        return m_currentHandContext->personalContext.cashAtHandStart;
    }
    return 0; // Default when no context
}

void Player::setLastMoneyWon(int theValue)
{
    if (m_currentHandContext)
    {
        m_currentHandContext->personalContext.lastMoneyWon = theValue;
    }
}
int Player::getLastMoneyWon() const
{
    if (m_currentHandContext)
    {
        return m_currentHandContext->personalContext.lastMoneyWon;
    }
    return 0; // Default when no context
}

void Player::setHoleCards(const HoleCards& cards)
{
    if (m_currentHandContext)
    {
        m_currentHandContext->personalContext.holeCards = cards;
    }
}

void Player::setHoleCards(const Card& card1, const Card& card2)
{
    if (m_currentHandContext)
    {
        m_currentHandContext->personalContext.holeCards = HoleCards(card1, card2);
    }
}

std::string Player::getCardsValueString() const
{
    return getHoleCards().toString();
}

const HandSimulationStats Player::computeHandSimulation() const
{
#if (False)
    const int nbOpponents = m_seatsList->size() - 1;
    // evaluate my strength against my opponents's guessed ranges :
    float maxOpponentsStrengths = getMaxOpponentsStrengths();
    return m_handEvaluator->simulateHandEquity(getCardsValueString(), getStringBoard(), nbOpponents,
                                                       maxOpponentsStrengths);
#else
    return m_handEvaluator->simulateHandEquity("As 6d", "", 2, 0.5);
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
    float result = 0;

    auto opponent = *getPlayerListIteratorById(m_currentHandContext->commonContext.playersContext.actingPlayersList, opponentId);

    assert(getHandRanking() > 0);

    // compute winning hands % against my rank
    int nbWinningHands = 0;

    if (opponent->m_rangeEstimator->getEstimatedRange().size() == 0)
    {
        opponent->getRangeEstimator()->computeEstimatedPreflopRange(*m_currentHandContext);
    }

    vector<std::string> ranges = RangeParser::getRangeAtomicValues(opponent->m_rangeEstimator->getEstimatedRange());

    vector<std::string> newRanges;

    for (vector<std::string>::const_iterator i = ranges.begin(); i != ranges.end(); i++)
    {

        if ((*i).size() != 4)
        {
            if (m_logger) {
                m_logger->error("invalid hand : " + (*i));
            }
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
        if (opponentId != m_id)
        {
            const std::string card1Str = getHoleCards().card1.toString();
            const std::string card2Str = getHoleCards().card2.toString();
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
        const int rank = m_handEvaluator->rankHand(((*i) + board).c_str());
        if (rank > getHandRanking())
        {
            nbWinningHands++;
        }
    }
    if (ranges.size() == 0)
    {
        if (m_logger) {
            m_logger->error("no ranges for opponent " + std::to_string(opponentId));
        }
        return 0;
    }
    assert(nbWinningHands / ranges.size() <= 1.0);
    return (float) nbWinningHands / (float) newRanges.size();

}

std::map<int, float> Player::evaluateOpponentsStrengths() const
{

    map<int, float> result;

    PlayerList players = m_currentHandContext->commonContext.playersContext.actingPlayersList;
    const int nbPlayers = players->size();

    for (PlayerListIterator it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getId() == m_id || (*it)->getLastAction().type == ActionType::Fold ||
            (*it)->getLastAction().type == ActionType::None)
        {
            continue;
        }

        const float estimatedOpponentWinningHands = getOpponentWinningHandsPercentage((*it)->getId(),  m_currentHandContext->commonContext.stringBoard);
        assert(estimatedOpponentWinningHands <= 1.0);

        result[(*it)->getId()] = estimatedOpponentWinningHands;
    }

    return result;
}

bool Player::isInVeryLooseMode(const int nbPlayers) const
{
    int clampedPlayers = nbPlayers;
    if (clampedPlayers < 0)
        clampedPlayers = 0;
    if (clampedPlayers > MAX_NUMBER_OF_PLAYERS)
        clampedPlayers = MAX_NUMBER_OF_PLAYERS;
    PlayerStatistics stats = m_statisticsUpdater->getStatistics(clampedPlayers);

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

void Player::updateCurrentHandContext(Hand& currentHand)
{
    // common context
    m_currentHandContext->commonContext = currentHand.updateHandCommonContext();

    // Calculate valid actions for this specific player
    m_currentHandContext->commonContext.validActions = pkt::core::getValidActionsForPlayer(currentHand, m_id);

    // Player-specific, visible from the opponents :
    m_currentHandContext->personalContext.totalBetAmount =
        m_currentHandContext->personalContext.actions.currentHandActions.getHandTotalBetAmount();
    m_currentHandContext->personalContext.m = static_cast<int>(currentHand.getM(getCash()));
    m_currentHandContext->personalContext.hasPosition = hasPosition(getPosition(), currentHand.getActingPlayersList());

    if (m_currentHandContext->commonContext.playersContext.preflopLastRaiser)
    {
        m_currentHandContext->personalContext.actions.preflopIsAggressor =
            m_currentHandContext->commonContext.gameState == Preflop &&
            m_currentHandContext->commonContext.playersContext.preflopLastRaiser->getId() == m_id;
    }
    if (m_currentHandContext->commonContext.playersContext.flopLastRaiser)
    {
        m_currentHandContext->personalContext.actions.flopIsAggressor =
            m_currentHandContext->commonContext.gameState == Flop &&
            m_currentHandContext->commonContext.playersContext.flopLastRaiser->getId() == m_id;
    }
    if (m_currentHandContext->commonContext.playersContext.turnLastRaiser)
    {
        m_currentHandContext->personalContext.actions.turnIsAggressor =
            m_currentHandContext->commonContext.gameState == Turn &&
            m_currentHandContext->commonContext.playersContext.turnLastRaiser->getId() == m_id;
    }

    m_currentHandContext->personalContext.statistics =
        m_statisticsUpdater->getStatistics(m_currentHandContext->commonContext.playersContext.nbPlayers);
    m_currentHandContext->personalContext.id = m_id;
    m_currentHandContext->personalContext.actions.isInVeryLooseMode =
        isInVeryLooseMode(m_currentHandContext->commonContext.playersContext.nbPlayers);

    // player specific, hidden from the opponents :
    m_currentHandContext->personalContext.preflopCallingRange = calculatePreflopCallingRange(*m_currentHandContext);
    // guess what the opponents might have and evaluate our strength, given our hole cards and the board cards (if any)
    m_currentHandContext->personalContext.m_handSimulation = computeHandSimulation();
    m_currentHandContext->personalContext.postFlopAnalysisFlags = getPostFlopAnalysisFlags();
}

float Player::calculatePreflopCallingRange(const CurrentHandContext& ctx) const
{    
    PreflopRangeCalculator calculator(*m_logger, *m_randomizer);
    return calculator.calculatePreflopCallingRange(ctx);
}

void Player::resetForNewHand(const Hand& hand)
{
    m_currentHandContext->personalContext.actions.currentHandActions.reset();
    setCashAtHandStart(getCash());
    setPosition(hand);
    getRangeEstimator()->setEstimatedRange("");
}
void Player::processAction(const PlayerAction& action, Hand& hand)
{
    setAction(hand.getStateManager()->getCurrentState(), action);
    updateCurrentHandContext(hand);
    getStatisticsUpdater()->updateStatistics(hand.getStateManager()->getGameState(), getCurrentHandContext());
    getRangeEstimator()->updateUnplausibleRanges(hand.getStateManager()->getGameState(), getCurrentHandContext());
}
const PostFlopAnalysisFlags Player::getPostFlopAnalysisFlags() const
{
    std::string stringHand = getCardsValueString();
    std::string stringBoard = m_currentHandContext->commonContext.stringBoard;

    return m_handEvaluator->analyzeHand(getCardsValueString(), stringBoard);
}

void Player::setPosition(const Hand& hand)
{
    const int dealerId = hand.getDealerPlayerId();
    const PlayerList players = hand.getSeatsList();
    const int nbPlayers = players->size();

    // Compute relative offset clockwise from dealer
    int offset = playerDistanceCircularOffset(dealerId, m_id, players);
    PlayerPosition position = computePlayerPositionFromOffset(offset, nbPlayers);

    // Update context only
    if (m_currentHandContext)
    {
        m_currentHandContext->personalContext.position = position;
    }

    assert(position != Unknown);
}

void Player::setAction(HandState& state, const PlayerAction& action)
{
    if (action.type != ActionType::None)
    {
        if (m_logger) {
            m_logger->info(m_name + " " + std::string(actionTypeToString(action.type)) +
                                      (action.amount ? " " + std::to_string(action.amount) : ""));
        }
    }
    m_currentHandContext->personalContext.actions.currentHandActions.addAction(state.getGameState(), action);
}

const PlayerStatistics& Player::getStatistics(const int nbPlayers) const
{
    return m_statisticsUpdater->getStatistics(nbPlayers);
}

} // namespace pkt::core::player
