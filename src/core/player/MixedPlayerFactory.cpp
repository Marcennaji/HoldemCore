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

// ISP-compliant constructor
MixedPlayerFactory::MixedPlayerFactory(const GameEvents& events, StrategyAssigner* assigner,
                                       std::shared_ptr<pkt::core::Logger> logger,
                                       std::shared_ptr<pkt::core::HandEvaluationEngine> handEvaluator,
                                       std::shared_ptr<pkt::core::PlayersStatisticsStore> statisticsStore,
                                       std::shared_ptr<pkt::core::Randomizer> randomizer)
    : m_events(events), m_strategyAssigner(assigner), m_logger(logger), m_handEvaluator(handEvaluator), m_statisticsStore(statisticsStore), m_randomizer(randomizer)
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
    
    // Enforce strict ISP compliance - all interfaces must be available
    assert(m_logger && "Logger interface must be provided for ISP compliance");
    assert(m_handEvaluator && "HandEvaluationEngine interface must be provided for ISP compliance");
    assert(m_statisticsStore && "PlayersStatisticsStore interface must be provided for ISP compliance");
    assert(m_randomizer && "Randomizer interface must be provided for ISP compliance");
    
    // Use fully ISP-compliant constructor
    auto player = std::make_shared<Player>(m_events, m_logger, m_handEvaluator, m_statisticsStore, m_randomizer, id, "Human", startMoney);
    
    player->setStrategy(std::move(humanStrategy));
    return player;
}

std::shared_ptr<Player> MixedPlayerFactory::createBotPlayer(int id, TableProfile profile, int startMoney)
{
    auto strategy = m_strategyAssigner->chooseBotStrategyFor(id);
    
    // Enforce strict ISP compliance - all interfaces must be available
    assert(m_logger && "Logger service must be available for ISP-compliant Player creation");
    assert(m_handEvaluator && "HandEvaluationEngine service must be available for ISP-compliant Player creation");
    assert(m_statisticsStore && "StatisticsStore service must be available for ISP-compliant Player creation");
    assert(m_randomizer && "Randomizer service must be available for ISP-compliant Player creation");
    
    auto player = std::make_shared<Player>(m_events, m_logger, m_handEvaluator, m_statisticsStore, m_randomizer, id, "Bot_" + std::to_string(id), startMoney);
    
    player->setStrategy(std::move(strategy));
    return player;
}

} // namespace pkt::core::player