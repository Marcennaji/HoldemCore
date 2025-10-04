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

namespace pkt::core
{

EngineFactory::EngineFactory(const GameEvents& events) : m_events(events)
{
}

// ISP-compliant constructor using focused service interfaces
EngineFactory::EngineFactory(const GameEvents& events, 
                             std::shared_ptr<HasLogger> logger,
                             std::shared_ptr<HasHandEvaluationEngine> handEvaluator)
    : m_events(events), m_logger(logger), m_handEvaluator(handEvaluator)
{
}

EngineFactory::~EngineFactory() = default;

void EngineFactory::ensureServicesInitialized() const
{
    if (!m_services)
    {
        m_services = std::make_shared<AppServiceContainer>();
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
