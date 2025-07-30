#include "FlopState.h"
#include "GameEvents.h"
#include "HandFsm.h"
#include "core/engine/model/PlayerAction.h"

namespace pkt::core
{

FlopState::FlopState(const GameEvents& events) : myEvents(events)
{
}

void FlopState::enter(HandFsm& hand)
{
    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(GameStateFlop);
}

void FlopState::exit(HandFsm& hand)
{
}

std::unique_ptr<IHandState> FlopState::computeNextState(HandFsm& hand, PlayerAction action)
{
    return nullptr;
}

bool FlopState::isRoundComplete(const HandFsm& hand) const
{
    return false;
}

bool FlopState::isActionAllowed(const HandFsm& hand, PlayerAction action) const
{
    return false;
}

void FlopState::logStateInfo(const HandFsm& hand) const
{
    // Optional debug logging
}

} // namespace pkt::core