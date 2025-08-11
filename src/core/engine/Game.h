// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/player/typedefs.h>
#include "EngineDefs.h"
#include "model/GameData.h"
#include "model/StartData.h"

#include <list>
#include <memory>
#include <string>

namespace pkt::core
{

struct GameEvents;
class IHand;
class IBoard;
class EngineFactory;

class Game
{

  public:
    Game(const GameEvents&, std::shared_ptr<EngineFactory> factory, const pkt::core::player::PlayerList& playersList,
         const GameData& gameData, const StartData& startData);

    ~Game();

    void startNewHand();

  private:
    std::shared_ptr<EngineFactory> myEngineFactory;

    const GameEvents& myEvents;
    std::shared_ptr<IHand> myCurrentHand;
    std::shared_ptr<IBoard> myCurrentBoard;

    pkt::core::player::PlayerList mySeatsList;
    pkt::core::player::PlayerList myRunningPlayersList; // nonfolded and nonallin active players

    unsigned myDealerPlayerId{0};
    GameData myGameData;
    StartData myStartData;
};

} // namespace pkt::core
