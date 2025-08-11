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
    Game(const GameEvents& events, std::shared_ptr<EngineFactory> factory, std::shared_ptr<IBoard> board,
         pkt::core::player::PlayerList seatsList, unsigned dealerId, const GameData& gameData,
         const StartData& startData);

    ~Game();

    void startNewHand();

  private:
    void resetPlayerActions();
    void resetRunningPlayers();
    void createNewHand();
    void findNextDealer();

  private:
    std::shared_ptr<EngineFactory> myEngineFactory;
    const GameEvents& myEvents;
    std::shared_ptr<IHand> myCurrentHand;
    std::shared_ptr<IBoard> myCurrentBoard;

    pkt::core::player::PlayerList mySeatsList;
    pkt::core::player::PlayerList myRunningPlayersList;

    unsigned myDealerPlayerId{0};
    GameData myGameData;
    StartData myStartData;
};

} // namespace pkt::core
