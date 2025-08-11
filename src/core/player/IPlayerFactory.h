// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once
#include <memory>
#include "core/engine/model/TableProfile.h"
#include "core/interfaces/ILogger.h"
#include "core/player/deprecated/Player.h"

namespace pkt::core::player
{

class IPlayerFactory
{
  public:
    virtual ~IPlayerFactory() = default;

    virtual std::shared_ptr<Player> createHumanPlayer(int id, int startMoney) = 0;
    virtual std::shared_ptr<Player> createBotPlayer(int id, TableProfile profile, int startMoney) = 0;

  protected:
};

} // namespace pkt::core::player
