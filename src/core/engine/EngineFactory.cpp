// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "EngineFactory.h"

#include <core/services/ServiceContainer.h>
#include "core/engine/game/Board.h"
#include "strategy/LooseAggressiveBotStrategy.h"
#include "strategy/ManiacBotStrategy.h"
#include "strategy/TightAggressiveBotStrategy.h"
#include "core/interfaces/services/ServiceProviders.h"

namespace pkt::core
{

EngineFactory::EngineFactory(const GameEvents& events) : m_events(events), m_services(nullptr)
{
}

EngineFactory::EngineFactory(const GameEvents& events, std::shared_ptr<PokerServices> services)
    : m_events(events), m_services(services)
{
}

EngineFactory::EngineFactory(const GameEvents& events, 
                             std::shared_ptr<LoggerProvider> loggerProvider,
                             std::shared_ptr<HandEvaluationProvider> handEvalProvider)
    : m_events(events), m_services(nullptr), m_loggerProvider(loggerProvider), m_handEvalProvider(handEvalProvider)
{
}

EngineFactory::~EngineFactory() = default;

void EngineFactory::ensureServicesInitialized()
{
    if (!m_services)
    {
        auto baseContainer = std::make_shared<AppServiceContainer>();
        m_services = std::make_shared<PokerServices>(baseContainer);
    }
}

Logger& EngineFactory::getLogger()
{
    if (m_loggerProvider) {
        return m_loggerProvider->getLogger();
    }
    ensureServicesInitialized();
    return m_services->logger();
}

HandEvaluationEngine& EngineFactory::getHandEvaluationEngine()
{
    if (m_handEvalProvider) {
        return m_handEvalProvider->getHandEvaluationEngine();
    }
    ensureServicesInitialized();
    return m_services->handEvaluationEngine();
}

std::shared_ptr<Hand> EngineFactory::createHand(std::shared_ptr<EngineFactory> f, std::shared_ptr<Board> b,
                                                pkt::core::player::PlayerList seats,
                                                pkt::core::player::PlayerList actingPlayers, GameData gd, StartData sd)
{
    // Use legacy path if focused dependencies not available
    if (!m_loggerProvider || !m_handEvalProvider) {
        ensureServicesInitialized();
        return std::make_shared<Hand>(m_events, f, b, seats, actingPlayers, gd, sd, m_services);
    }
    
    // TODO: Update Hand constructor to accept focused dependencies
    // For now, create adapter and use legacy constructor
    ensureServicesInitialized();
    return std::make_shared<Hand>(m_events, f, b, seats, actingPlayers, gd, sd, m_services);
}

std::shared_ptr<Board> EngineFactory::createBoard(unsigned dealerPosition)
{
    // Use legacy path if focused dependencies not available
    if (!m_loggerProvider || !m_handEvalProvider) {
        ensureServicesInitialized();
        return std::make_shared<Board>(dealerPosition, m_events, m_services);
    }
    
    // TODO: Update Board constructor to accept focused dependencies
    // For now, create adapter and use legacy constructor
    ensureServicesInitialized();
    return std::make_shared<Board>(dealerPosition, m_events, m_services);
}
} // namespace pkt::core
