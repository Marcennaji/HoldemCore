// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <memory>
#include "core/engine/model/TableProfile.h"
#include "core/player/strategy/StrategyAssigner.h"
#include "core/ports/Logger.h"
#include "core/ports/HandEvaluationEngine.h"
#include "core/ports/PlayersStatisticsStore.h"
#include "core/ports/Randomizer.h"
#include "core/player/Player.h"

namespace pkt::core::player
{

/**
 * @brief Factory class for creating and configuring poker players.
 * 
 * Handles the creation of player instances with proper dependency injection,
 * strategy assignment, and initialization of player statistics and state
 * management components.
 */
class PlayerFactory
{
  public:
 
    PlayerFactory(const GameEvents& events, StrategyAssigner* assigner,
                  pkt::core::Logger& logger,
                  pkt::core::HandEvaluationEngine& handEvaluator,
                  pkt::core::PlayersStatisticsStore& statisticsStore,
                  pkt::core::Randomizer& randomizer);

    std::shared_ptr<Player> createPlayer(int id, TableProfile profile, int startMoney);

  private:
    const GameEvents& m_events;
    StrategyAssigner* m_strategyAssigner;
    
    pkt::core::Logger& m_logger;
    pkt::core::HandEvaluationEngine& m_handEvaluator;
    pkt::core::PlayersStatisticsStore& m_statisticsStore;
    pkt::core::Randomizer& m_randomizer;
    
    std::shared_ptr<Player> createHumanPlayer(int id, int startMoney);
    std::shared_ptr<Player> createBotPlayer(int id, TableProfile profile, int startMoney);
};

} // namespace pkt::core::player