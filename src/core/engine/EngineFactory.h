// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "EngineFactory.h"

#include "core/engine/hand/Hand.h"
#include "core/engine/model/GameData.h"
#include "core/engine/model/StartData.h"
#include "core/engine/game/Board.h"
#include "core/player/Player.h"
#include "core/services/PokerServices.h"

#include <memory>
#include <vector>

#include "core/player/typedefs.h"

namespace pkt::core
{
class EngineFactory
{
  public:
    EngineFactory(const GameEvents&);

    // Constructor with PokerServices for dependency injection
    EngineFactory(const GameEvents&, std::shared_ptr<PokerServices> services);

    ~EngineFactory();

    virtual std::shared_ptr<Hand> createHand(std::shared_ptr<EngineFactory> f, std::shared_ptr<Board> b,
                                             pkt::core::player::PlayerList seats,
                                             pkt::core::player::PlayerList actingPlayers, GameData, StartData);

    virtual std::shared_ptr<Board> createBoard(unsigned dealerPosition);

  private:
    const GameEvents& myEvents;
    std::shared_ptr<PokerServices> myServices; // Injected service container

    void ensureServicesInitialized();
};

} // namespace pkt::core
