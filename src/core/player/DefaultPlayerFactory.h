// PokerTraining — Texas Hold'em simulator
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

    std::shared_ptr<Player> createHumanPlayer(int id, int startMoney) override;
    std::shared_ptr<Player> createBotPlayer(int id, TableProfile profile, int startMoney) override;
    std::shared_ptr<PlayerFsm> createPlayerFsm(int id, TableProfile profile, int startMoney) override;

  private:
    const GameEvents& myEvents;
    StrategyAssigner* myStrategyAssigner;
};
} // namespace pkt::core::player
