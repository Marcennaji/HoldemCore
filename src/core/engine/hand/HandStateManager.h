#pragma once

#include <functional>
#include <memory>
#include "core/engine/model/GameState.h"

namespace pkt::core
{
class ServiceContainer;
class Logger;
} // namespace pkt::core

namespace pkt::core
{

class Hand;
class HandState;
class HandActionProcessor;
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
    explicit HandStateManager(const GameEvents& events, int smallBlind, unsigned dealerPlayerId,
                              GameLoopErrorCallback errorCallback,
                              std::shared_ptr<pkt::core::ServiceContainer> services);
    
    // ISP-compliant constructor with focused Logger service (preferred)
    HandStateManager(const GameEvents& events, int smallBlind, unsigned int dealerPlayerId,
                     GameLoopErrorCallback errorCallback, std::shared_ptr<Logger> logger);
    ~HandStateManager() = default;

    // State lifecycle management
    void initializeState(Hand& hand);
    void runGameLoop(Hand& hand);
    void handleStateTransition(Hand& hand);
    void terminateGame(Hand& hand);

    // State queries
    bool isTerminal() const;
    GameState getGameState() const;
    HandActionProcessor* getActionProcessor() const;
    HandState& getCurrentState() const;

    // Force state transition (used after player actions)
    void transitionToNextState(Hand& hand);

  private:
    void transitionState(Hand& hand, std::unique_ptr<HandState> newState);
    void checkAndHandleTerminalState(Hand& hand);
    void ensureServicesInitialized() const;

    const GameEvents& m_events;
    std::unique_ptr<HandState> m_currentState;
    GameLoopErrorCallback m_errorCallback;
    mutable std::shared_ptr<pkt::core::ServiceContainer> m_services; // Legacy support
    std::shared_ptr<Logger> m_logger; // ISP-compliant focused service

    // State initialization parameters
    int m_smallBlind;
    unsigned m_dealerPlayerId;

    // Game loop protection
    static const int MAX_GAME_LOOP_ITERATIONS = 1000;
};

} // namespace pkt::core