// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "PlayerFactory.h"
#include "core/player/Player.h"
#include "core/player/strategy/BotStrategy.h"
#include "core/player/strategy/HumanStrategy.h"

namespace pkt::core::player
{

PlayerFactory::PlayerFactory(const GameEvents& events, StrategyAssigner* assigner,
                              pkt::core::Logger& logger,
                              pkt::core::HandEvaluationEngine& handEvaluator,
                              pkt::core::PlayersStatisticsStore& statisticsStore,
                              pkt::core::Randomizer& randomizer)
    : m_events(events), m_strategyAssigner(assigner), m_logger(&logger), m_handEvaluator(&handEvaluator), m_statisticsStore(&statisticsStore), m_randomizer(&randomizer)
{
}

std::shared_ptr<Player> PlayerFactory::createPlayer(int id, TableProfile profile, int startMoney)
{
    if (id == 0) {
        // Player 0 is always the human player
        return createHumanPlayer(id, startMoney);
    } else {
        // All other players are bots
        return createBotPlayer(id, profile, startMoney);
    }
}

std::shared_ptr<Player> PlayerFactory::createHumanPlayer(int id, int startMoney)
{
    auto humanStrategy = std::make_unique<HumanStrategy>(m_events);
    auto player = std::make_shared<Player>(m_events, *m_logger, *m_handEvaluator, *m_statisticsStore, *m_randomizer, id, "Human", startMoney);
    
    player->setStrategy(std::move(humanStrategy));
    return player;
}

std::shared_ptr<Player> PlayerFactory::createBotPlayer(int id, TableProfile profile, int startMoney)
{
    auto strategy = m_strategyAssigner->chooseBotStrategyFor(id);    
    auto player = std::make_shared<Player>(m_events, *m_logger, *m_handEvaluator, *m_statisticsStore, *m_randomizer, id, "Bot_" + std::to_string(id), startMoney);
    
    player->setStrategy(std::move(strategy));
    return player;
}

} // namespace pkt::core::player