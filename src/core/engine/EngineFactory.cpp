// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "EngineFactory.h"

#include <core/services/ServiceContainer.h>
#include "core/engine/game/Board.h"
#include "strategy/LooseAggressiveBotStrategy.h"
#include "strategy/ManiacBotStrategy.h"
#include "strategy/TightAggressiveBotStrategy.h"

namespace pkt::core
{

EngineFactory::EngineFactory(const GameEvents& events) : m_events(events), m_services(nullptr)
{
}

EngineFactory::EngineFactory(const GameEvents& events, std::shared_ptr<PokerServices> services)
    : m_events(events), m_services(services)
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

std::shared_ptr<Hand> EngineFactory::createHand(std::shared_ptr<EngineFactory> f, std::shared_ptr<Board> b,
                                                pkt::core::player::PlayerList seats,
                                                pkt::core::player::PlayerList actingPlayers, GameData gd, StartData sd)
{
    ensureServicesInitialized();
    return std::make_shared<Hand>(m_events, f, b, seats, actingPlayers, gd, sd, m_services);
}

std::shared_ptr<Board> EngineFactory::createBoard(unsigned dealerPosition)
{
    ensureServicesInitialized();
    return std::make_shared<Board>(dealerPosition, m_events, m_services);
}
} // namespace pkt::core
