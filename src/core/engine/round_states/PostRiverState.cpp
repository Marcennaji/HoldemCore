#include "PostRiverState.h"
#include "GameEvents.h"
#include "Hand.h"
#include "core/engine/model/PlayerAction.h"

namespace pkt::core
{

PostRiverState::PostRiverState(GameEvents& events) : myEvents(events)
{
}

void PostRiverState::enter(IHand& hand)
{
    // hand.revealHands();
    // hand.getBoard()->determineWinners();

    hand.getBoard()->distributePot();

    if (myEvents.onShowdownStarted)
    {
        myEvents.onShowdownStarted();
    }
}

void PostRiverState::exit(IHand& hand)
{
    // Nothing needed
}

std::unique_ptr<IBettingRoundStateFsm> PostRiverState::processAction(IHand& hand, PlayerAction action)
{
    // No actions processed in this state
    return nullptr;
}

GameState PostRiverState::getGameState() const
{
    return GameState::GameStatePostRiver;
}

std::string PostRiverState::getStateName() const
{
    return "PostRiver";
}

bool PostRiverState::isRoundComplete(const IHand& hand) const
{
    return true; // This is terminal
}

bool PostRiverState::canProcessAction(const IHand& hand, PlayerAction action) const
{
    return false; // No actions allowed here
}

void PostRiverState::logStateInfo(const IHand& hand) const
{
    // Optional debug logging
}

} // namespace pkt::core