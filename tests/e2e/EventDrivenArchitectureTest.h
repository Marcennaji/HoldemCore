#pragma once

#include <set>
#include <vector>
#include "common/EngineTest.h"
#include "core/engine/GameEvents.h"
#include "core/engine/model/GameState.h"
#include "core/engine/model/PlayerAction.h"

namespace pkt::test
{

/**
 * MockUI - Acts as an external UI system to validate hexagonal architecture
 *
 * This class simulates how a real UI would interact with the poker engine.
 * In proper hexagonal architecture, the UI should:
 * 1. Never call engine methods directly to get state information
 * 2. Receive all necessary information through events/callbacks
 * 3. Only send commands to the engine (actions, start game, etc.)
 */
class MockUI
{
  public:
    // Event data collection
    struct PlayerTurnData
    {
        unsigned playerId;
        core::GameState gameState;
        std::vector<core::ActionType> validActions;
        int currentBet = 0;
        int chipStack = 0;
        bool isHuman = false;
    };

    struct GameStateData
    {
        core::GameState currentState;
        int potSize = 0;
        std::vector<unsigned> activePlayerIds;
        unsigned currentPlayerId = 0;
    };

    // Collected events
    std::vector<core::GameState> bettingRoundsStarted;
    std::vector<core::PlayerAction> playerActions;
    std::vector<PlayerTurnData> playerTurns;
    std::vector<GameStateData> gameStates;
    std::vector<std::pair<unsigned, int>> chipUpdates;
    std::vector<int> potUpdates;

    // Error event tracking
    struct InvalidActionData
    {
        unsigned playerId;
        core::PlayerAction action;
        std::string reason;
    };
    std::vector<InvalidActionData> invalidActions;
    std::vector<std::string> engineErrors;

    bool awaitingHumanInput = false;
    bool gameInitialized = false;
    int gameSpeed = 0;

    void clear();

    // Analysis methods - what a real UI would need to know
    bool canDetermineCurrentPlayer() const;
    bool hasValidActionsForCurrentPlayer() const;
    bool canDisplayGameState() const;
    unsigned getCurrentPlayerFromEvents() const;
};

class EventDrivenArchitectureTest : public EngineTest
{
  protected:
    void SetUp() override;
    void createGameEvents();

    std::unique_ptr<MockUI> mockUI;
};

} // namespace pkt::test