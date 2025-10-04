// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/engine/hand/HandEvaluator.h>
#include "core/engine/GameEvents.h"

#include "core/engine/cards/Card.h"
#include "core/engine/model/PlayerPosition.h"
#include "core/player/PlayerStatisticsUpdater.h"
#include "core/player/strategy/PlayerStrategy.h"

#include <string>
#include <typeinfo>
#include "CurrentHandActions.h"
#include "range/RangeEstimator.h"
#include "strategy/CurrentHandContext.h"

#include "core/interfaces/HandEvaluationEngine.h"
#include "core/interfaces/Logger.h"
#include "core/services/ServiceContainer.h"
#include "core/interfaces/HasLogger.h"
#include "core/interfaces/HasHandEvaluationEngine.h"

#include <array>
#include <assert.h>
#include <filesystem>
#include <map>
#include <memory>
#include <string>

namespace pkt::core
{
class Hand;
class HandState;

} // namespace pkt::core

namespace pkt::core::player
{

class Player
{
  public:
    Player(const GameEvents&, int id, std::string name, int cash);
    Player(const GameEvents&, std::shared_ptr<ServiceContainer> services, int id, std::string name, int cash);
    // ISP-compliant constructor using focused service interfaces
    Player(const GameEvents&, std::shared_ptr<HasLogger> logger, std::shared_ptr<HasHandEvaluationEngine> handEvaluator, int id, std::string name, int cash);
    virtual ~Player() = default;

    // ========================================
    // Core Identity & Properties
    // ========================================
    int getId() const;
    std::string getName() const;

    // ========================================
    // Strategy Management (Delegates to m_strategy)
    // ========================================
    void setStrategy(std::unique_ptr<PlayerStrategy> strategy) { m_strategy = std::move(strategy); }

    PlayerAction decideAction(const CurrentHandContext& ctx)
    {
        assert(m_strategy && "PlayerStrategy must be set before deciding action");
        return m_strategy->decideAction(ctx);
    }

    std::string getStrategyTypeName() const
    {
        if (!m_strategy)
        {
            return "None";
        }
        const auto& strategy = *m_strategy;
        return typeid(strategy).name();
    }

    template <typename StrategyType> bool hasStrategyType() const
    {
        if (!m_strategy)
        {
            return false;
        }
        return dynamic_cast<const StrategyType*>(m_strategy.get()) != nullptr;
    }

    // ========================================
    // Context Access & Management
    // ========================================
    CurrentHandContext& getCurrentHandContext() { return *m_currentHandContext; }
    void updateCurrentHandContext(Hand&);
    void resetForNewHand(const Hand& hand);
    void processAction(const PlayerAction& action, Hand& hand);

    // ========================================
    // State Delegates (Delegate to m_currentHandContext)
    // ========================================
    // Cash management
    void setCash(int theValue);
    int getCash() const;
    void addBetAmount(int theValue);
    void setCashAtHandStart(int theValue);
    int getCashAtHandStart() const;
    void setLastMoneyWon(int theValue);
    int getLastMoneyWon() const;

    // Cards & position
    const HoleCards& getHoleCards() const;
    void setHoleCards(const HoleCards& cards);
    void setHoleCards(const Card& card1, const Card& card2);
    std::string getCardsValueString() const;
    const PlayerPosition getPosition() const;
    void setPosition(const Hand& hand);

    // Hand evaluation & ranking
    void setHandRanking(int theValue);
    int getHandRanking() const;
    const PostFlopAnalysisFlags getPostFlopAnalysisFlags() const;

    // Actions & gameplay
    void setAction(HandState& state, const PlayerAction& action);
    PlayerAction getLastAction() const;
    const CurrentHandActions& getCurrentHandActions() const;
    void resetCurrentHandActions();

    // ========================================
    // Statistics Delegates (Delegate to m_statisticsUpdater)
    // ========================================
    const PlayerStatistics& getStatistics(const int nbPlayers) const;
    const std::unique_ptr<PlayerStatisticsUpdater>& getStatisticsUpdater() const { return m_statisticsUpdater; }

    // ========================================
    // Range Estimation Delegates (Delegate to m_rangeEstimator)
    // ========================================
    virtual float calculatePreflopCallingRange(const CurrentHandContext& ctx) const;
    std::unique_ptr<RangeEstimator>& getRangeEstimator() { return m_rangeEstimator; }

    // ========================================
    // Service Access Helpers (ISP-compliant)
    // ========================================
    pkt::core::Logger& getLogger() const;
    pkt::core::HandEvaluationEngine& getHandEvaluationEngine() const;

    // ========================================
    // Business Logic Methods (Actual Implementation)
    // ========================================
    bool isInVeryLooseMode(const int nbPlayers) const;

  protected:
    float getMaxOpponentsStrengths() const;
    float getOpponentWinningHandsPercentage(const int idPlayer, std::string board) const;

    const GameEvents& m_events;
    std::unique_ptr<CurrentHandContext> m_currentHandContext;
    const int m_id;
    const std::string m_name;
    std::unique_ptr<RangeEstimator> m_rangeEstimator;

  private:
    void ensureServicesInitialized() const;

    std::map<int, float> evaluateOpponentsStrengths() const;
    const HandSimulationStats computeHandSimulation() const;
    std::unique_ptr<PlayerStrategy> m_strategy;
    std::unique_ptr<PlayerStatisticsUpdater> m_statisticsUpdater;
    mutable std::shared_ptr<ServiceContainer> m_services; // Injected service container (legacy)
    // ISP-compliant focused service interfaces
    std::shared_ptr<HasLogger> m_logger;
    std::shared_ptr<HasHandEvaluationEngine> m_handEvaluator;
};
} // namespace pkt::core::player
