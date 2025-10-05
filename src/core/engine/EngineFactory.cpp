// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "EngineFactory.h"
#include "../interfaces/ServiceAdapter.h"
#include "../services/PokerServices.h"

#include <core/services/ServiceContainer.h>
#include "core/engine/game/Board.h"
#include "strategy/LooseAggressiveBotStrategy.h"
#include "strategy/ManiacBotStrategy.h"
#include "strategy/TightAggressiveBotStrategy.h"
#include "core/interfaces/services/ServiceProviders.h"
#include "core/engine/EngineDefs.h"

// Adapter classes to bridge ISP interfaces back to concrete implementations
// These are needed during the migration period to populate ServiceContainer
namespace {

class LoggerFromISP : public pkt::core::Logger {
public:
    explicit LoggerFromISP(std::shared_ptr<pkt::core::HasLogger> logger) : m_logger(logger) {}
    
    void setLogLevel(pkt::core::LogLevel level) override {
        m_logger->logger().setLogLevel(level);
    }
    
    void debug(const std::string& message) override {
        m_logger->logger().debug(message);
    }
    
    void info(const std::string& message) override {
        m_logger->logger().info(message);
    }
    
    void verbose(const std::string& message) override {
        m_logger->logger().verbose(message);
    }
    
    void error(const std::string& message) override {
        m_logger->logger().error(message);
    }

private:
    std::shared_ptr<pkt::core::HasLogger> m_logger;
};

class HandEvaluationEngineFromISP : public pkt::core::HandEvaluationEngine {
public:
    explicit HandEvaluationEngineFromISP(std::shared_ptr<pkt::core::HasHandEvaluationEngine> engine) 
        : m_engine(engine) {}
    
    unsigned int rankHand(const char* hand) override {
        return m_engine->handEvaluationEngine().rankHand(hand);
    }
    
    pkt::core::HandSimulationStats simulateHandEquity(const std::string& hand, const std::string& board,
                                                      const int nbOpponents, float maxOpponentsStrengths) override {
        return m_engine->handEvaluationEngine().simulateHandEquity(hand, board, nbOpponents, maxOpponentsStrengths);
    }
    
    pkt::core::PostFlopAnalysisFlags analyzeHand(const std::string& hand, const std::string& board) override {
        return m_engine->handEvaluationEngine().analyzeHand(hand, board);
    }
    
    const char* getEngineName() const override {
        return m_engine->handEvaluationEngine().getEngineName();
    }

private:
    std::shared_ptr<pkt::core::HasHandEvaluationEngine> m_engine;
};

class PlayersStatisticsStoreFromISP : public pkt::core::PlayersStatisticsStore {
public:
    explicit PlayersStatisticsStoreFromISP(std::shared_ptr<pkt::core::HasPlayersStatisticsStore> store) 
        : m_store(store) {}
    
    std::array<pkt::core::player::PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1>
    loadPlayerStatistics(const std::string& playerName) override {
        return m_store->playersStatisticsStore().loadPlayerStatistics(playerName);
    }
    
    void savePlayersStatistics(pkt::core::player::PlayerList seatsList) override {
        m_store->playersStatisticsStore().savePlayersStatistics(seatsList);
    }

private:
    std::shared_ptr<pkt::core::HasPlayersStatisticsStore> m_store;
};

} // anonymous namespace

namespace pkt::core
{

EngineFactory::EngineFactory(const GameEvents& events) : m_events(events)
{
}

// ISP-compliant constructor using focused service interfaces
EngineFactory::EngineFactory(const GameEvents& events, 
                             std::shared_ptr<HasLogger> logger,
                             std::shared_ptr<HasHandEvaluationEngine> handEvaluator,
                             std::shared_ptr<HasPlayersStatisticsStore> statisticsStore)
    : m_events(events), m_logger(logger), m_handEvaluator(handEvaluator), m_statisticsStore(statisticsStore)
{
}

EngineFactory::~EngineFactory() = default;

void EngineFactory::ensureServicesInitialized() const
{
    if (!m_services)
    {
        auto appServices = std::make_shared<AppServiceContainer>();
        m_services = appServices;
        
        // If we have ISP interfaces, populate the service container with compatible implementations
        if (m_logger) {
            // Create a logger that delegates to our ISP interface
            auto loggerImpl = std::make_unique<LoggerFromISP>(m_logger);
            appServices->setLogger(std::move(loggerImpl));
        }
        
        if (m_handEvaluator) {
            // Create a hand evaluator that delegates to our ISP interface  
            auto evaluatorImpl = std::make_unique<HandEvaluationEngineFromISP>(m_handEvaluator);
            appServices->setHandEvaluationEngine(std::move(evaluatorImpl));
        }
        
        if (m_statisticsStore) {
            // Create a statistics store that delegates to our ISP interface
            auto storeImpl = std::make_unique<PlayersStatisticsStoreFromISP>(m_statisticsStore);
            appServices->setPlayersStatisticsStore(std::move(storeImpl));
        }
    }
}

pkt::core::Logger& EngineFactory::getLogger() const
{
    if (m_logger) {
        return m_logger->logger();
    }
    // Fallback to legacy service container
    ensureServicesInitialized();
    return m_services->logger();
}

pkt::core::HandEvaluationEngine& EngineFactory::getHandEvaluationEngine() const
{
    if (m_handEvaluator) {
        return m_handEvaluator->handEvaluationEngine();
    }
    // Fallback to legacy service container
    ensureServicesInitialized();
    return m_services->handEvaluationEngine();
}

std::shared_ptr<pkt::core::ServiceContainer> EngineFactory::getServiceContainer() const
{
    ensureServicesInitialized();
    return m_services;
}

std::shared_ptr<Hand> EngineFactory::createHand(std::shared_ptr<EngineFactory> f, std::shared_ptr<Board> b,
                                                pkt::core::player::PlayerList seats,
                                                pkt::core::player::PlayerList actingPlayers, GameData gd, StartData sd)
{
    // Use ISP-compliant approach when focused dependencies are available
    if (m_logger && m_handEvaluator) {
        // Create PokerServices wrapper for legacy Hand constructor
        ensureServicesInitialized(); // Ensure m_services is available
        auto pokerServices = std::make_shared<PokerServices>(m_services);
        return std::make_shared<Hand>(m_events, f, b, seats, actingPlayers, gd, sd, pokerServices);
    }
    
    // Legacy fallback
    ensureServicesInitialized();
    auto pokerServices = std::make_shared<PokerServices>(m_services);
    return std::make_shared<Hand>(m_events, f, b, seats, actingPlayers, gd, sd, pokerServices);
}

std::shared_ptr<Board> EngineFactory::createBoard(unsigned dealerPosition)
{
    // Use ISP-compliant approach when focused dependencies are available
    if (m_logger && m_handEvaluator) {
        // Use legacy ServiceContainer for Board constructor
        ensureServicesInitialized(); // Ensure m_services is available
        return std::make_shared<Board>(dealerPosition, m_events, m_services);
    }
    
    // Legacy fallback
    ensureServicesInitialized();
    return std::make_shared<Board>(dealerPosition, m_events, m_services);
}
} // namespace pkt::core
