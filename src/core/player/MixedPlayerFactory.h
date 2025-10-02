// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "IPlayerFactory.h"
#include "core/player/strategy/StrategyAssigner.h"

namespace pkt::core::player
{

class MixedPlayerFactory : public IPlayerFactory
{
  public:
    MixedPlayerFactory(const GameEvents& events, StrategyAssigner* assigner);

    std::shared_ptr<Player> createPlayer(int id, TableProfile profile, int startMoney) override;

  private:
    const GameEvents& m_events;
    StrategyAssigner* m_strategyAssigner;
    
    std::shared_ptr<Player> createHumanPlayer(int id, int startMoney);
    std::shared_ptr<Player> createBotPlayer(int id, TableProfile profile, int startMoney);
};

} // namespace pkt::core::player