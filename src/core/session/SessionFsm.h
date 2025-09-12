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
#include "core/interfaces/IBoard.h"

namespace pkt::core
{
class GameFsm;
class EngineFactory;
namespace player
{
class StrategyAssigner;
}
class IBoard;

class SessionFsm
{
  public:
    SessionFsm(const GameEvents& events);

    // Constructor for dependency injection (testing)
    SessionFsm(const GameEvents& events, std::shared_ptr<EngineFactory> engineFactory);

    ~SessionFsm();

    void startGame(const GameData& gameData, const StartData& startData);

  protected:
    // Virtual methods for testability
    virtual std::unique_ptr<player::StrategyAssigner> createStrategyAssigner(const TableProfile& tableProfile,
                                                                             int numberOfBots);

    virtual std::unique_ptr<player::DefaultPlayerFactory>
    createPlayerFactory(const GameEvents& events, player::StrategyAssigner* strategyAssigner);

    virtual std::shared_ptr<IBoard> createBoard(const StartData& startData);

  private:
    pkt::core::player::PlayerFsmList createPlayersList(player::DefaultPlayerFactory& playerFactory, int numberOfPlayers,
                                                       unsigned startMoney, const TableProfile& tableProfile);

    std::unique_ptr<GameFsm> myCurrentGame;
    GameEvents myEvents;
    std::shared_ptr<EngineFactory> myEngineFactory; // Injected or created
};

} // namespace pkt::core
