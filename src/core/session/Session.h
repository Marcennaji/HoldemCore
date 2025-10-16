// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <memory>

#include <core/engine/model/BotGameData.h>
#include <core/engine/model/GameData.h>
#include <core/engine/model/PlayerAction.h>
#include <core/engine/model/StartData.h>
#include <core/player/PlayerFactory.h>
#include <core/player/typedefs.h>
#include "core/engine/GameEvents.h"
#include "core/engine/game/Board.h"
#include "core/ports/HandEvaluationEngine.h"
#include "core/ports/Logger.h"
#include "core/ports/PlayersStatisticsStore.h"
#include "core/ports/Randomizer.h"

namespace pkt::core
{
class Game;
class EngineFactory;
namespace player
{
class StrategyAssigner;
}
class Board;

/**
 * @brief Manages poker game sessions and coordinates game lifecycle.
 *
 * Orchestrates the overall game session including player management,
 * hand execution, action handling, and coordination between different
 * game components like board, players, and statistics.
 */
class Session
{
  public:
    Session(const GameEvents& events, EngineFactory& engineFactory, Logger& logger,
            HandEvaluationEngine& handEvaluationEngine, PlayersStatisticsStore& playersStatisticsStore,
            Randomizer& randomizer);

    ~Session();

    /**
     * @brief Start a 1 human player vs. n bots game, assigned random bot strategies from table profile)
     * @param gameData Game configuration
     * @param startData Game start parameters
     */
    void startGame(const GameData& gameData, const StartData& startData);

    /**
     * @brief Start a bot-only game with custom strategy distribution
     * @param botGameData Configuration including strategy distribution map
     * @param startData Game start parameters
     */
    void startBotOnlyGameWithCustomStrategies(const BotGameData& botGameData, const StartData& startData);

    void handlePlayerAction(const PlayerAction& action);

    void startNewHand();

  protected:
    // Virtual methods for testability
    virtual std::unique_ptr<player::StrategyAssigner> createStrategyAssigner(const TableProfile& tableProfile,
                                                                             int numberOfBots);

    virtual std::unique_ptr<player::PlayerFactory> createPlayerFactory(const GameEvents& events,
                                                                       player::StrategyAssigner* strategyAssigner);

    virtual std::shared_ptr<Board> createBoard(const StartData& startData);

    // Helper methods for startGame refactoring (protected for testing)
    void validateGameParameters(const GameData& gameData, const StartData& startData);
    virtual void validatePlayerConfiguration(const pkt::core::player::PlayerList& playersList);
    void fireGameInitializedEvent(int guiSpeed);

    // Protected members for derived classes
    Logger& m_logger;
    Randomizer& m_randomizer;
    const GameEvents& m_events;
    EngineFactory& m_engineFactory;
    HandEvaluationEngine& m_handEvaluationEngine;
    PlayersStatisticsStore& m_playersStatisticsStore;

    struct GameComponents
    {
        std::unique_ptr<player::StrategyAssigner> strategyAssigner;
        std::unique_ptr<player::PlayerFactory> playerFactory;
        pkt::core::player::PlayerList playersList;
        std::shared_ptr<Board> board;
    };

    virtual GameComponents createGameComponents(const GameData& gameData, const StartData& startData);

    GameComponents createCustomStrategyComponents(const BotGameData& botGameData, const StartData& startData);

    pkt::core::player::PlayerList createPlayersList(player::PlayerFactory& playerFactory, int numberOfPlayers,
                                                    unsigned startMoney, const TableProfile& tableProfile);

    void initializeGame(GameComponents&& components, const GameData& gameData, const StartData& startData);

  private:
    std::unique_ptr<Game> m_currentGame;
};

} // namespace pkt::core
