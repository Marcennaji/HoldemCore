// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "IPlayerFactory.h"
#include "core/player/strategy/StrategyAssigner.h"

namespace pkt::core::player
{

class DefaultPlayerFactory : public IPlayerFactory
{
  public:
    DefaultPlayerFactory(const GameEvents& events, StrategyAssigner* assigner);

    std::shared_ptr<Player> createPlayer(int id, TableProfile profile, int startMoney) override;

  private:
    const GameEvents& m_events;
    StrategyAssigner* m_strategyAssigner;
};
} // namespace pkt::core::player
