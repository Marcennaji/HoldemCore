// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "ISPObjectFactory.h"
#include "core/engine/state/FlopState.h"
#include "core/engine/state/RiverState.h"
#include "core/engine/state/TurnState.h"
#include "core/engine/state/PreflopState.h"
#include "core/engine/state/PostRiverState.h"
#include "core/player/strategy/ManiacBotStrategy.h"
#include "core/player/strategy/TightAggressiveBotStrategy.h"
#include "core/player/Player.h"

namespace pkt::core
{

std::shared_ptr<FlopState> ISPObjectFactory::createFlopState(const GameEvents& events) const
{
    auto loggerService = m_adapter.createLoggerService();
    return std::make_shared<FlopState>(events, loggerService);
}

std::shared_ptr<RiverState> ISPObjectFactory::createRiverState(const GameEvents& events) const
{
    auto loggerService = m_adapter.createLoggerService();
    return std::make_shared<RiverState>(events, loggerService);
}

std::shared_ptr<ManiacBotStrategy> ISPObjectFactory::createManiacBotStrategy() const
{
    auto loggerService = m_adapter.createLoggerService();
    auto randomizerService = m_adapter.createRandomizerService();
    return std::make_shared<ManiacBotStrategy>(loggerService, randomizerService);
}

std::shared_ptr<TightAggressiveBotStrategy> ISPObjectFactory::createTightAggressiveBotStrategy() const
{
    auto loggerService = m_adapter.createLoggerService();
    auto randomizerService = m_adapter.createRandomizerService();
    return std::make_shared<TightAggressiveBotStrategy>(loggerService, randomizerService);
}

std::shared_ptr<Player> ISPObjectFactory::createPlayer(const GameEvents& events, int id, const std::string& name, int cash) const
{
    auto loggerService = m_adapter.createLoggerService();
    auto handEvaluationService = m_adapter.createHandEvaluationEngineService();
    auto statisticsStoreService = m_adapter.createPlayersStatisticsStoreService();
    auto randomizerService = m_adapter.createRandomizerService();
    return std::make_shared<Player>(events, loggerService, handEvaluationService, statisticsStoreService, randomizerService, id, name, cash);
}

std::shared_ptr<TurnState> ISPObjectFactory::createTurnState(const GameEvents& events) const
{
    auto loggerService = m_adapter.createLoggerService();
    return std::make_shared<TurnState>(events, loggerService);
}

std::shared_ptr<PreflopState> ISPObjectFactory::createPreflopState(const GameEvents& events, int smallBlind, unsigned dealerPlayerId) const
{
    auto loggerService = m_adapter.createLoggerService();
    return std::make_shared<PreflopState>(events, smallBlind, dealerPlayerId, loggerService);
}

std::shared_ptr<PostRiverState> ISPObjectFactory::createPostRiverState(const GameEvents& events) const
{
    auto loggerService = m_adapter.createLoggerService();
    return std::make_shared<PostRiverState>(events, loggerService);
}

} // namespace pkt::core