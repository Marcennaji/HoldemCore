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
class Game;

class Session
{
  public:
    Session(const GameEvents& events);
    ~Session();

    void startGame(const GameData& gameData, const StartData& startData);

  private:
    pkt::core::player::PlayerList createPlayersList(player::DefaultPlayerFactory& playerFactory, int numberOfPlayers,
                                                    unsigned startMoney, const TableProfile& tableProfile);

  private:
    std::unique_ptr<Game> myCurrentGame;
    GameEvents myEvents;
};

} // namespace pkt::core
