// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "EngineFactory.h"
#include "../interfaces/ServiceAdapter.h"
#include "../services/PokerServices.h"

#include <core/services/ServiceContainer.h>
#include <core/services/DefaultRandomizer.h>
#include "core/engine/game/Board.h"
#include "strategy/LooseAggressiveBotStrategy.h"
#include "strategy/ManiacBotStrategy.h"
#include "strategy/TightAggressiveBotStrategy.h"
#include "core/interfaces/services/ServiceProviders.h"
#include "core/engine/EngineDefs.h"
#include <infra/ConsoleLogger.h>
#include <infra/eval/PsimHandEvaluationEngine.h>
#include <infra/persistence/SqlitePlayersStatisticsStore.h>
#include <stdexcept>

// Adapter classes to bridge ISP interfaces back to concrete implementations
// These are needed during the migration period to populate ServiceContainer
namespace {

// Wrapper classes are no longer needed since we use direct interfaces

} // anonymous namespace

namespace pkt::core
{

// Backward-compatible constructor for legacy code and tests
EngineFactory::EngineFactory(const GameEvents& events) 
    : m_events(events)
{
    // Create default services for backward compatibility
    // This maintains existing behavior while transitioning to ISP
    auto logger = std::make_shared<pkt::infra::ConsoleLogger>();
    logger->setLogLevel(pkt::core::LogLevel::Info);
    
    auto handEvaluator = std::make_shared<pkt::infra::PsimHandEvaluationEngine>();
    
    // For statistics store, we'll need a default implementation from ServiceContainer
    auto container = std::make_shared<AppServiceContainer>();
    
    m_logger = logger;
    m_handEvaluator = handEvaluator;
    // Use aliasing constructor to create shared_ptr that references the container's statistics store
    // but keeps the container alive through the control block
    m_statisticsStore = std::shared_ptr<PlayersStatisticsStore>(container, &container->playersStatisticsStore());
}

// ISP-compliant constructor with focused service interfaces (preferred)
EngineFactory::EngineFactory(const GameEvents& events, 
                             std::shared_ptr<Logger> logger,
                             std::shared_ptr<HandEvaluationEngine> handEvaluator,
                             std::shared_ptr<PlayersStatisticsStore> statisticsStore)
    : m_events(events), m_logger(logger), m_handEvaluator(handEvaluator), m_statisticsStore(statisticsStore)
{
    // Validate that all required dependencies are provided
    if (!m_logger || !m_handEvaluator || !m_statisticsStore) {
        throw std::invalid_argument("EngineFactory requires all focused dependencies (Logger, HandEvaluationEngine, PlayersStatisticsStore)");
    }
}

EngineFactory::~EngineFactory() = default;

// Helper method to create temporary ServiceContainer from focused services
// This is a bridge during migration - eventually Hand and Board should accept focused services directly
std::shared_ptr<pkt::core::ServiceContainer> EngineFactory::createServiceContainerFromFocusedServices() const
{
    auto container = std::make_shared<AppServiceContainer>();
    
    // Create simple copies of our focused services since ServiceContainer expects to own them
    auto loggerCopy = std::make_unique<pkt::infra::ConsoleLogger>();
    loggerCopy->setLogLevel(pkt::core::LogLevel::Info);
    
    auto handEvaluatorCopy = std::make_unique<pkt::infra::PsimHandEvaluationEngine>();
    
    container->setLogger(std::move(loggerCopy));
    container->setHandEvaluationEngine(std::move(handEvaluatorCopy));
    
    // TODO: Remove this bridge method entirely once Hand is converted to ISP
    // For now, let the container create its own default statistics store
    
    // We'll need a default randomizer since it's not part of our focused services yet
    container->setRandomizer(std::make_unique<DefaultRandomizer>());
    
    return container;
}

std::shared_ptr<Hand> EngineFactory::createHand(std::shared_ptr<EngineFactory> f, std::shared_ptr<Board> b,
                                                pkt::core::player::PlayerList seats,
                                                pkt::core::player::PlayerList actingPlayers, GameData gd, StartData sd)
{
    // Use ISP-compliant Hand constructor with focused services
    return std::make_shared<Hand>(m_events, f, b, seats, actingPlayers, gd, sd, m_logger, m_statisticsStore);
}

std::shared_ptr<Board> EngineFactory::createBoard(unsigned dealerPosition)
{
    // Create ServiceContainer from our focused services for legacy Board constructor
    auto serviceContainer = createServiceContainerFromFocusedServices();
    
    return std::make_shared<Board>(dealerPosition, m_events, serviceContainer);
}

// Legacy method for backward compatibility (used by Hand.cpp)
std::shared_ptr<pkt::core::ServiceContainer> EngineFactory::getServiceContainer() const
{
    return createServiceContainerFromFocusedServices();
}

} // namespace pkt::core
