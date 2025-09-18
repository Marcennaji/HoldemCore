#pragma once

#include <functional>
#include <memory>
#include "core/engine/model/GameState.h"

namespace pkt::core
{

class Hand;
class IHandState;
class IActionProcessor;
struct GameEvents;

/**
 * @brief HandStateManager manages game state transitions and state-specific logic
 *
 * This class centralizes all state management functionality that was previously
 * handled directly in the Hand class. It manages state creation, transitions,
 * queries, and the main game loop logic.
 */
class HandStateManager
{
  public:
    using GameLoopErrorCallback = std::function<void(const std::string&)>;

    HandStateManager(const GameEvents& events, int smallBlind, unsigned dealerPlayerId,
                     GameLoopErrorCallback errorCallback);
    ~HandStateManager() = default;

    // State lifecycle management
    void initializeState(Hand& hand);
    void runGameLoop(Hand& hand);
    void handleStateTransition(Hand& hand);
    void terminateGame(Hand& hand);

    // State queries
    bool isTerminal() const;
    GameState getGameState() const;
    IActionProcessor* getActionProcessor() const;
    IHandState& getCurrentState() const;

    // Force state transition (used after player actions)
    void transitionToNextState(Hand& hand);

  private:
    void transitionState(Hand& hand, std::unique_ptr<IHandState> newState);
    void checkAndHandleTerminalState(Hand& hand);

    const GameEvents& myEvents;
    std::unique_ptr<IHandState> myCurrentState;
    GameLoopErrorCallback myErrorCallback;

    // State initialization parameters
    int mySmallBlind;
    unsigned myDealerPlayerId;

    // Game loop protection
    static const int MAX_GAME_LOOP_ITERATIONS = 1000;
};

} // namespace pkt::core