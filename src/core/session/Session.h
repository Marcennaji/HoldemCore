// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <memory>

#include <core/engine/model/GameData.h>
#include <core/engine/model/StartData.h>
#include <core/engine/model/PlayerAction.h>
#include <core/player/DefaultPlayerFactory.h>
#include <core/player/MixedPlayerFactory.h>
#include <core/player/typedefs.h>
#include "core/engine/GameEvents.h"
#include "core/engine/game/Board.h"
#include "core/services/ServiceContainer.h"

namespace pkt::core
{
class Game;
class EngineFactory;
namespace player
{
class StrategyAssigner;
}
class Board;

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
    
    // Method to handle player actions from GUI
    void handlePlayerAction(const PlayerAction& action);
    
    // Method to start a new hand (called after hand completion)
    void startNewHand();

  protected:
    // Virtual methods for testability
    virtual std::unique_ptr<player::StrategyAssigner> createStrategyAssigner(const TableProfile& tableProfile,
                                                                             int numberOfBots);

    virtual std::unique_ptr<player::MixedPlayerFactory>
    createPlayerFactory(const GameEvents& events, player::StrategyAssigner* strategyAssigner);

    virtual std::shared_ptr<Board> createBoard(const StartData& startData);

    // Helper methods for startGame refactoring (protected for testing)
    void validateGameParameters(const GameData& gameData, const StartData& startData);
    void validatePlayerConfiguration(const pkt::core::player::PlayerList& playersList);
    void fireGameInitializedEvent(int guiSpeed);
    void ensureEngineFactoryInitialized();
    void ensureServiceContainerInitialized();

  private:
    pkt::core::player::PlayerList createPlayersList(player::MixedPlayerFactory& playerFactory, int numberOfPlayers,
                                                    unsigned startMoney, const TableProfile& tableProfile);

    struct GameComponents
    {
        std::unique_ptr<player::StrategyAssigner> strategyAssigner;
        std::unique_ptr<player::MixedPlayerFactory> playerFactory;
        pkt::core::player::PlayerList playersList;
        std::shared_ptr<Board> board;
    };

    GameComponents createGameComponents(const GameData& gameData, const StartData& startData);
    void initializeGame(GameComponents&& components, const GameData& gameData, const StartData& startData);

    std::unique_ptr<Game> m_currentGame;
    const GameEvents& m_events;
    std::shared_ptr<EngineFactory> m_engineFactory;       // Injected or created
    std::shared_ptr<ServiceContainer> m_serviceContainer; // Injected service container
};

} // namespace pkt::core
