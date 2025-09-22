// HoldemCore — Texas Hold'em simulator
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
#include "core/services/ServiceContainer.h"

namespace pkt::core
{
class Game;
class EngineFactory;
namespace player
{
class StrategyAssigner;
}
class IBoard;

class Session
{
  public:
    Session(const GameEvents& events);

    // Constructor for dependency injection (testing)
    Session(const GameEvents& events, std::shared_ptr<EngineFactory> engineFactory);

    // Constructor with ServiceContainer for proper dependency injection
    Session(const GameEvents& events, std::shared_ptr<ServiceContainer> serviceContainer);

    ~Session();

    void startGame(const GameData& gameData, const StartData& startData);

  protected:
    // Virtual methods for testability
    virtual std::unique_ptr<player::StrategyAssigner> createStrategyAssigner(const TableProfile& tableProfile,
                                                                             int numberOfBots);

    virtual std::unique_ptr<player::DefaultPlayerFactory>
    createPlayerFactory(const GameEvents& events, player::StrategyAssigner* strategyAssigner);

    virtual std::shared_ptr<IBoard> createBoard(const StartData& startData);

    // Helper methods for startGame refactoring (protected for testing)
    void validateGameParameters(const GameData& gameData, const StartData& startData);
    void fireGameInitializedEvent(int guiSpeed);
    void ensureEngineFactoryInitialized();
    void ensureServiceContainerInitialized();

  private:
    pkt::core::player::PlayerList createPlayersList(player::DefaultPlayerFactory& playerFactory, int numberOfPlayers,
                                                    unsigned startMoney, const TableProfile& tableProfile);

    struct GameComponents
    {
        std::unique_ptr<player::StrategyAssigner> strategyAssigner;
        std::unique_ptr<player::DefaultPlayerFactory> playerFactory;
        pkt::core::player::PlayerList playersList;
        std::shared_ptr<IBoard> board;
    };

    GameComponents createGameComponents(const GameData& gameData, const StartData& startData);
    void initializeGame(GameComponents&& components, const GameData& gameData, const StartData& startData);

    std::unique_ptr<Game> myCurrentGame;
    const GameEvents& myEvents;
    std::shared_ptr<EngineFactory> myEngineFactory;       // Injected or created
    std::shared_ptr<ServiceContainer> myServiceContainer; // Injected service container
};

} // namespace pkt::core
