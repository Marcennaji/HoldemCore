// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <memory>

#include <core/engine/model/GameData.h>
#include <core/engine/model/StartData.h>
#include <core/player/DefaultPlayerFactory.h>
#include <core/player/typedefs.h>
#include "core/engine/GameEvents.h"
namespace pkt::core
{
class GameFsm;

class SessionFsm
{
  public:
    SessionFsm(const GameEvents& events);
    ~SessionFsm();

    void startGame(const GameData& gameData, const StartData& startData);

  private:
    pkt::core::player::PlayerFsmList createPlayersList(player::DefaultPlayerFactory& playerFactory, int numberOfPlayers,
                                                       unsigned startMoney, const TableProfile& tableProfile);

  private:
    std::unique_ptr<GameFsm> myCurrentGame;
    GameEvents myEvents;
};

} // namespace pkt::core
