// PokerTraining — Texas Hold'em simulator
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
#include "core/services/ServiceContainer.h"

#include <array>
#include <assert.h>
#include <filesystem>
#include <map>
#include <memory>
#include <string>

namespace pkt::core
{
class Hand;
class IHandState;

} // namespace pkt::core

namespace pkt::core::player
{

class Player
{
  public:
    Player(const GameEvents&, int id, std::string name, int cash);
    Player(const GameEvents&, std::shared_ptr<ServiceContainer> services, int id, std::string name, int cash);
    virtual ~Player() = default;

    // ========================================
    // Core Identity & Properties
    // ========================================
    int getId() const;
    std::string getName() const;

    // ========================================
    // Strategy Management (Delegates to myStrategy)
    // ========================================
    void setStrategy(std::unique_ptr<PlayerStrategy> strategy) { myStrategy = std::move(strategy); }

    PlayerAction decideAction(const CurrentHandContext& ctx)
    {
        assert(myStrategy && "PlayerStrategy must be set before deciding action");
        return myStrategy->decideAction(ctx);
    }

    std::string getStrategyTypeName() const
    {
        if (!myStrategy)
        {
            return "None";
        }
        const auto& strategy = *myStrategy;
        return typeid(strategy).name();
    }

    template <typename StrategyType> bool hasStrategyType() const
    {
        if (!myStrategy)
        {
            return false;
        }
        return dynamic_cast<const StrategyType*>(myStrategy.get()) != nullptr;
    }

    // ========================================
    // Context Access & Management
    // ========================================
    CurrentHandContext& getCurrentHandContext() { return *myCurrentHandContext; }
    void updateCurrentHandContext(Hand&);
    void resetForNewHand(const Hand& hand);
    void processAction(const PlayerAction& action, Hand& hand);

    // ========================================
    // State Delegates (Delegate to myCurrentHandContext)
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
    void setAction(IHandState& state, const PlayerAction& action);
    PlayerAction getLastAction() const;
    const CurrentHandActions& getCurrentHandActions() const;
    void resetCurrentHandActions();

    // ========================================
    // Statistics Delegates (Delegate to myStatisticsUpdater)
    // ========================================
    const PlayerStatistics& getStatistics(const int nbPlayers) const;
    const std::unique_ptr<PlayerStatisticsUpdater>& getStatisticsUpdater() const { return myStatisticsUpdater; }

    // ========================================
    // Range Estimation Delegates (Delegate to myRangeEstimator)
    // ========================================
    virtual float calculatePreflopCallingRange(const CurrentHandContext& ctx) const;
    std::unique_ptr<RangeEstimator>& getRangeEstimator() { return myRangeEstimator; }

    // ========================================
    // Business Logic Methods (Actual Implementation)
    // ========================================
    bool isInVeryLooseMode(const int nbPlayers) const;

  protected:
    float getMaxOpponentsStrengths() const;
    float getOpponentWinningHandsPercentage(const int idPlayer, std::string board) const;

    const GameEvents& myEvents;
    std::unique_ptr<CurrentHandContext> myCurrentHandContext;
    const int myID;
    const std::string myName;
    std::unique_ptr<RangeEstimator> myRangeEstimator;

  private:
    void ensureServicesInitialized() const;

    std::map<int, float> evaluateOpponentsStrengths() const;
    const HandSimulationStats computeHandSimulation() const;
    std::unique_ptr<PlayerStrategy> myStrategy;
    std::unique_ptr<PlayerStatisticsUpdater> myStatisticsUpdater;
    mutable std::shared_ptr<ServiceContainer> myServices; // Injected service container
};
} // namespace pkt::core::player
