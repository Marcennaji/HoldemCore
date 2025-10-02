// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "MixedPlayerFactory.h"
#include "core/player/Player.h"
#include "core/player/strategy/BotStrategy.h"
#include "core/player/strategy/HumanStrategy.h"

namespace pkt::core::player
{

MixedPlayerFactory::MixedPlayerFactory(const GameEvents& events, StrategyAssigner* assigner)
    : m_events(events), m_strategyAssigner(assigner)
{
}

std::shared_ptr<Player> MixedPlayerFactory::createPlayer(int id, TableProfile profile, int startMoney)
{
    if (id == 0) {
        // Player 0 is always the human player
        return createHumanPlayer(id, startMoney);
    } else {
        // All other players are bots
        return createBotPlayer(id, profile, startMoney);
    }
}

std::shared_ptr<Player> MixedPlayerFactory::createHumanPlayer(int id, int startMoney)
{
    auto humanStrategy = std::make_unique<HumanStrategy>(m_events);
    auto player = std::make_shared<Player>(m_events, id, "Human", startMoney);
    player->setStrategy(std::move(humanStrategy));
    return player;
}

std::shared_ptr<Player> MixedPlayerFactory::createBotPlayer(int id, TableProfile profile, int startMoney)
{
    auto strategy = m_strategyAssigner->chooseBotStrategyFor(id);
    auto player = std::make_shared<Player>(m_events, id, "Bot_" + std::to_string(id), startMoney);
    player->setStrategy(std::move(strategy));
    return player;
}

} // namespace pkt::core::player