#include "HandStateManager.h"
#include "GameEvents.h"
#include "Hand.h"
#include "core/engine/state/PreflopState.h"
#include "core/engine/hand/ActionProcessor.h"
#include "core/engine/hand/HandState.h"
#include "core/interfaces/Logger.h"

#include <cassert>

namespace pkt::core
{

HandStateManager::HandStateManager(const GameEvents& events, int smallBlind, unsigned int dealerPlayerId,
                                   GameLoopErrorCallback errorCallback, Logger& logger)
    : m_events(events), m_errorCallback(std::move(errorCallback)), m_smallBlind(smallBlind),
      m_dealerPlayerId(dealerPlayerId), m_logger(&logger)
{
}



void HandStateManager::initializeState(Hand& hand)
{
    // Direct ISP usage - no service aggregates needed
    m_currentState = std::make_unique<PreflopState>(m_events, m_smallBlind, m_dealerPlayerId, *m_logger);
    m_currentState->enter(hand);
}

void HandStateManager::runGameLoop(Hand& hand)
{
    int iterationCount = 0;

    while (!isTerminal() && iterationCount < MAX_GAME_LOOP_ITERATIONS)
    {
        iterationCount++;

        if (auto* processor = dynamic_cast<HandActionProcessor*>(m_currentState.get()))
        {
            auto nextPlayer = processor->getNextPlayerToAct(hand);
            if (nextPlayer)
            {
                processor->promptPlayerAction(hand, *nextPlayer);
            }
            else
            {
                // No next player to act, round should be complete
                // Transition to next state (e.g., flop -> turn)
                auto next = processor->computeNextState(hand);
                assert(next);
                transitionState(hand, std::move(next));
            }
        }
    }

    // Check if we hit the emergency brake
    if (iterationCount >= MAX_GAME_LOOP_ITERATIONS)
    {
        m_logger->error("Game loop hit maximum iterations (" + std::to_string(MAX_GAME_LOOP_ITERATIONS) +
                        "), terminating to prevent infinite loop");

        if (m_errorCallback)
        {
            m_errorCallback("Game loop terminated due to possible infinite loop");
        }

        // Force terminate the game
        return;
    }

    checkAndHandleTerminalState(hand);
}

void HandStateManager::handleStateTransition(Hand& hand)
{
    // This method is called after a player action to check if state transition is needed
    if (auto* processor = getActionProcessor())
    {
        auto next = processor->computeNextState(hand);
        if (next)
        {
            transitionState(hand, std::move(next));
            checkAndHandleTerminalState(hand);
        }
    }
}

void HandStateManager::transitionToNextState(Hand& hand)
{
    // Force transition to next state (used after player actions)
    handleStateTransition(hand);
}

void HandStateManager::terminateGame(Hand& hand)
{
    if (m_currentState)
    {
        m_currentState->exit(hand);
    }
    hand.end();
}

bool HandStateManager::isTerminal() const
{
    return m_currentState ? m_currentState->isTerminal() : true;
}

GameState HandStateManager::getGameState() const
{
    return m_currentState ? m_currentState->getGameState() : GameState::PostRiver;
}

HandActionProcessor* HandStateManager::getActionProcessor() const
{
    return dynamic_cast<HandActionProcessor*>(m_currentState.get());
}

HandState& HandStateManager::getCurrentState() const
{
    assert(m_currentState);
    return *m_currentState;
}

void HandStateManager::transitionState(Hand& hand, std::unique_ptr<HandState> newState)
{
    if (m_currentState)
    {
        m_currentState->exit(hand);
    }
    m_currentState = std::move(newState);
    m_currentState->enter(hand);
}

void HandStateManager::checkAndHandleTerminalState(Hand& hand)
{
    if (isTerminal())
    {
        terminateGame(hand);
    }
}

} // namespace pkt::core