// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "EngineFactory.h"

#include "core/engine/HandFsm.h"
#include "core/engine/model/GameData.h"
#include "core/engine/model/StartData.h"
#include "core/interfaces/IBoard.h"
#include "core/player/PlayerFsm.h"

#include <memory>
#include <vector>

#include "core/player/typedefs.h"

namespace pkt::core
{
class EngineFactory
{
  public:
    EngineFactory(const GameEvents&);
    ~EngineFactory();

    virtual std::shared_ptr<HandFsm> createHandFsm(std::shared_ptr<EngineFactory> f, std::shared_ptr<IBoard> b,
                                                   pkt::core::player::PlayerFsmList seats,
                                                   pkt::core::player::PlayerFsmList actingPlayers, GameData, StartData);

    virtual std::shared_ptr<IBoard> createBoardFsm(unsigned dealerPosition);

  private:
    const GameEvents& myEvents;
};

} // namespace pkt::core
