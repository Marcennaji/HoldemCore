// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/ServiceAdapter.h"
#include "core/interfaces/Logger.h"
#include "core/interfaces/Randomizer.h"
#include "core/interfaces/HandEvaluationEngine.h"
#include "core/interfaces/persistence/PlayersStatisticsStore.h"
#include "core/services/ServiceContainer.h"
#include <memory>

namespace pkt::core
{

/**
 * @brief Factory for creating ISP-compliant objects with focused dependencies
 * 
 * This factory follows SOLID principles:
 * - Single Responsibility: Only responsible for creating ISP-compliant objects
 * - Open/Closed: Can be extended with new creation methods without modification
 * - Dependency Inversion: Depends on abstractions (focused interfaces), not concrete classes
 * 
 * Makes it easy to create objects with focused dependencies during migration.
 */
class ISPObjectFactory
{
  public:
    explicit ISPObjectFactory(std::shared_ptr<ServiceContainer> services) 
        : m_adapter(services) {}
    
    // === State Creation Methods (ISP-Compliant) ===
    
    template<typename StateType>
    std::shared_ptr<StateType> createLoggerOnlyState(const GameEvents& events) const
    {
        static_assert(std::is_constructible_v<StateType, const GameEvents&, std::shared_ptr<Logger>>, 
                     "StateType must have constructor accepting GameEvents and Logger");
        
        auto loggerService = m_adapter.createLoggerService();
        return std::make_shared<StateType>(events, loggerService);
    }
    
    // === Strategy Creation Methods (ISP-Compliant) ===
    
    template<typename StrategyType>
    std::shared_ptr<StrategyType> createLoggerRandomizerStrategy() const
    {
        static_assert(std::is_constructible_v<StrategyType, std::shared_ptr<Logger>, std::shared_ptr<Randomizer>>, 
                     "StrategyType must have constructor accepting Logger and Randomizer");
        
        auto loggerService = m_adapter.createLoggerService();
        auto randomizerService = m_adapter.createRandomizerService();
        return std::make_shared<StrategyType>(loggerService, randomizerService);
    }
    
    // === Complex Object Creation Methods ===
    
    template<typename ObjectType>
    std::shared_ptr<ObjectType> createLoggerHandEvaluationObject(auto&&... args) const
    {
        auto loggerService = m_adapter.createLoggerService();
        auto handEvalService = m_adapter.createHandEvaluationEngineService();
        return std::make_shared<ObjectType>(std::forward<decltype(args)>(args)..., loggerService, handEvalService);
    }
    
    // === Convenience Methods for Common Patterns ===
    
    /**
     * @brief Create FlopState with focused logging dependency
     * 
     * Example:
     *   auto flopState = factory.createFlopState(gameEvents);
     */
    std::shared_ptr<class FlopState> createFlopState(const GameEvents& events) const;
    
    /**
     * @brief Create RiverState with focused logging dependency
     */
    std::shared_ptr<class RiverState> createRiverState(const GameEvents& events) const;
    
    /**
     * @brief Create ManiacBotStrategy with focused dependencies
     */
    std::shared_ptr<class ManiacBotStrategy> createManiacBotStrategy() const;
    
    /**
     * @brief Create TightAggressiveBotStrategy with focused dependencies (Logger + Randomizer)
     */
    std::shared_ptr<class TightAggressiveBotStrategy> createTightAggressiveBotStrategy() const;
    
    /**
     * @brief Create Player with focused dependencies (Logger + HandEvaluationEngine)
     */
    std::shared_ptr<class Player> createPlayer(const GameEvents& events, int id, const std::string& name, int cash) const;
    
    /**
     * @brief Create TurnState with focused Logger dependency
     */
    std::shared_ptr<class TurnState> createTurnState(const GameEvents& events) const;
    
    /**
     * @brief Create PreflopState with focused Logger dependency
     */
    std::shared_ptr<class PreflopState> createPreflopState(const GameEvents& events, int smallBlind, unsigned dealerPlayerId) const;
    
    /**
     * @brief Create PostRiverState with focused Logger dependency
     */
    std::shared_ptr<class PostRiverState> createPostRiverState(const GameEvents& events) const;
    
    // === Raw Focused Services (for custom creation) ===
    
    std::shared_ptr<Logger> getLoggerService() const {
        return m_adapter.createLoggerService();
    }
    
    std::shared_ptr<Randomizer> getRandomizerService() const {
        return m_adapter.createRandomizerService();
    }
    
    std::shared_ptr<HandEvaluationEngine> getHandEvaluationEngineService() const {
        return m_adapter.createHandEvaluationEngineService();
    }
    
    std::shared_ptr<PlayersStatisticsStore> getPlayersStatisticsStoreService() const {
        return m_adapter.createPlayersStatisticsStoreService();
    }
    
  private:
    ServiceAdapter m_adapter;
};

} // namespace pkt::core