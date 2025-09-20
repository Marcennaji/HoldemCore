#include "HandStateManager.h"
#include "GameEvents.h"
#include "Hand.h"
#include "core/engine/state/PreflopState.h"
#include "core/interfaces/hand/IActionProcessor.h"
#include "core/interfaces/hand/IHandState.h"
#include "core/services/ServiceContainer.h"

#include <cassert>

namespace pkt::core
{

HandStateManager::HandStateManager(const GameEvents& events, int smallBlind, unsigned dealerPlayerId,
                                   GameLoopErrorCallback errorCallback)
    : myEvents(events), myErrorCallback(std::move(errorCallback)), mySmallBlind(smallBlind),
      myDealerPlayerId(dealerPlayerId)
{
}

HandStateManager::HandStateManager(const GameEvents& events, int smallBlind, unsigned dealerPlayerId,
                                   GameLoopErrorCallback errorCallback,
                                   std::shared_ptr<pkt::core::ServiceContainer> services)
    : myEvents(events), myErrorCallback(std::move(errorCallback)), mySmallBlind(smallBlind),
      myDealerPlayerId(dealerPlayerId), myServices(std::move(services))
{
}

void HandStateManager::ensureServicesInitialized() const
{
    if (!myServices)
    {
        myServices = std::make_shared<pkt::core::AppServiceContainer>();
    }
}

void HandStateManager::initializeState(Hand& hand)
{
    myCurrentState = std::make_unique<PreflopState>(myEvents, mySmallBlind, myDealerPlayerId);
    myCurrentState->enter(hand);
}

void HandStateManager::runGameLoop(Hand& hand)
{
    ensureServicesInitialized();
    int iterationCount = 0;

    while (!isTerminal() && iterationCount < MAX_GAME_LOOP_ITERATIONS)
    {
        iterationCount++;

        if (auto* processor = dynamic_cast<IActionProcessor*>(myCurrentState.get()))
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
        myServices->logger().error("Game loop hit maximum iterations (" + std::to_string(MAX_GAME_LOOP_ITERATIONS) +
                                   "), terminating to prevent infinite loop");

        if (myErrorCallback)
        {
            myErrorCallback("Game loop terminated due to possible infinite loop");
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
    if (myCurrentState)
    {
        myCurrentState->exit(hand);
    }
    hand.end();
}

bool HandStateManager::isTerminal() const
{
    return myCurrentState ? myCurrentState->isTerminal() : true;
}

GameState HandStateManager::getGameState() const
{
    return myCurrentState ? myCurrentState->getGameState() : GameState::PostRiver;
}

IActionProcessor* HandStateManager::getActionProcessor() const
{
    return dynamic_cast<IActionProcessor*>(myCurrentState.get());
}

IHandState& HandStateManager::getCurrentState() const
{
    assert(myCurrentState);
    return *myCurrentState;
}

void HandStateManager::transitionState(Hand& hand, std::unique_ptr<IHandState> newState)
{
    if (myCurrentState)
    {
        myCurrentState->exit(hand);
    }
    myCurrentState = std::move(newState);
    myCurrentState->enter(hand);
}

void HandStateManager::checkAndHandleTerminalState(Hand& hand)
{
    if (isTerminal())
    {
        terminateGame(hand);
    }
}

} // namespace pkt::core