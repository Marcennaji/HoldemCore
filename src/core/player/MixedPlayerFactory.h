// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "IPlayerFactory.h"
#include "core/player/strategy/StrategyAssigner.h"
#include "core/interfaces/Logger.h"
#include "core/interfaces/HandEvaluationEngine.h"
#include "core/interfaces/persistence/PlayersStatisticsStore.h"
#include "core/interfaces/Randomizer.h"

namespace pkt::core::player
{

class MixedPlayerFactory : public IPlayerFactory
{
  public:
    MixedPlayerFactory(const GameEvents& events, StrategyAssigner* assigner);
    
    // ISP-compliant constructor
    MixedPlayerFactory(const GameEvents& events, StrategyAssigner* assigner,
                       std::shared_ptr<pkt::core::Logger> logger,
                       std::shared_ptr<pkt::core::HandEvaluationEngine> handEvaluator,
                       std::shared_ptr<pkt::core::PlayersStatisticsStore> statisticsStore,
                       std::shared_ptr<pkt::core::Randomizer> randomizer);

    std::shared_ptr<Player> createPlayer(int id, TableProfile profile, int startMoney) override;

  private:
    const GameEvents& m_events;
    StrategyAssigner* m_strategyAssigner;
    
    // ISP-compliant service interfaces
    std::shared_ptr<pkt::core::Logger> m_logger;
    std::shared_ptr<pkt::core::HandEvaluationEngine> m_handEvaluator;
    std::shared_ptr<pkt::core::PlayersStatisticsStore> m_statisticsStore;
    std::shared_ptr<pkt::core::Randomizer> m_randomizer;
    
    std::shared_ptr<Player> createHumanPlayer(int id, int startMoney);
    std::shared_ptr<Player> createBotPlayer(int id, TableProfile profile, int startMoney);
};

} // namespace pkt::core::player