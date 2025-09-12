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
class HandFsm;
class IBoard;
class EngineFactory;

class GameFsm
{
  public:
    GameFsm(const GameEvents& events, std::shared_ptr<EngineFactory> factory, std::shared_ptr<IBoard> board,
            pkt::core::player::PlayerFsmList seatsList, unsigned dealerId, const GameData& gameData,
            const StartData& startData);

    ~GameFsm() = default;

    void startNewHand();

  private:
    void findNextDealer();

  private:
    std::shared_ptr<EngineFactory> myEngineFactory;
    const GameEvents& myEvents;
    std::shared_ptr<HandFsm> myCurrentHand;
    std::shared_ptr<IBoard> myCurrentBoard;

    pkt::core::player::PlayerFsmList mySeatsList;
    pkt::core::player::PlayerFsmList myActingPlayersList;

    unsigned myDealerPlayerId{0};
    GameData myGameData;
    StartData myStartData;
};

} // namespace pkt::core
