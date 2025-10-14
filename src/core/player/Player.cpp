// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Player.h"

#include <core/engine/cards/CardUtilities.h>
#include <core/engine/hand/Hand.h>
#include <core/engine/hand/HandEvaluator.h>
#include <core/engine/model/Ranges.h>
#include <core/engine/utils/Helpers.h>
#include <core/player/range/RangeParser.h>
#include <core/player/strategy/CurrentHandContext.h>
#include <core/player/strategy/PreflopRangeCalculator.h>
#include <core/ports/PlayersStatisticsStore.h>
#include "Helpers.h"
#include "core/ports/Logger.h"

#include <sstream>

namespace pkt::core::player
{

using namespace std;

// Fully ISP-compliant constructor with all required interfaces
Player::Player(const GameEvents& events, pkt::core::Logger& logger, pkt::core::HandEvaluationEngine& handEvaluator,
               pkt::core::PlayersStatisticsStore& statisticsStore, pkt::core::Randomizer& randomizer, int id,
               std::string name, int cash)
    : m_id(id), m_name(name), m_events(events), m_logger(logger), m_handEvaluator(handEvaluator),
      m_statisticsStore(statisticsStore), m_randomizer(randomizer)
{
    m_rangeEstimator = std::make_unique<RangeEstimator>(m_id, logger, handEvaluator);
    m_opponentsStrengthsEvaluator = std::make_unique<OpponentsStrengthsEvaluator>(m_id, logger, handEvaluator);
    m_currentHandContext = std::make_unique<CurrentHandContext>();
    m_statisticsUpdater = std::make_unique<PlayerStatisticsUpdater>(statisticsStore);
    m_statisticsUpdater->loadStatistics(getStrategyName());

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
    const int nbOpponents = m_currentHandContext->commonContext.playersContext.nbPlayers - 1;
    // Evaluate strength against opponents' guessed ranges
    auto evaluation =
        m_opponentsStrengthsEvaluator->evaluateOpponents(*m_currentHandContext, getHoleCards(), getHandRanking());
    return m_handEvaluator.simulateHandEquity(getCardsValueString(), m_currentHandContext->commonContext.stringBoard,
                                              nbOpponents, evaluation.maxStrength);
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
    PreflopRangeCalculator calculator(m_logger, m_randomizer);
    return calculator.calculatePreflopCallingRange(ctx);
}

void Player::resetForNewHand(const Hand& hand)
{
    m_currentHandContext->personalContext.actions.currentHandActions.reset();
    setCashAtHandStart(getCash());
    setPosition(hand);
    getRangeEstimator()->setEstimatedRange("");
    // Clear hole cards from previous hand
    m_currentHandContext->personalContext.holeCards = HoleCards();
    m_currentHandContext->personalContext.handRanking = 0;
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

    return m_handEvaluator.analyzeHand(getCardsValueString(), stringBoard);
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
        m_logger.info(m_name + " " + std::string(actionTypeToString(action.type)) +
                      (action.amount ? " " + std::to_string(action.amount) : ""));
    }
    m_currentHandContext->personalContext.actions.currentHandActions.addAction(state.getGameState(), action);
}

const PlayerStatistics& Player::getStatistics(const int nbPlayers) const
{
    return m_statisticsUpdater->getStatistics(nbPlayers);
}

} // namespace pkt::core::player
