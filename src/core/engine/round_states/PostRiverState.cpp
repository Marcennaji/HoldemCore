#include "PostRiverState.h"
#include "GameEvents.h"
#include "HandFsm.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/Helpers.h"
#include "core/player/PlayerFsm.h"

namespace pkt::core
{
using namespace pkt::core::player;

PostRiverState::PostRiverState(const GameEvents& events) : myEvents(events)
{
}

void PostRiverState::enter(HandFsm& hand)
{
    // Final state entry logic: typically showdown or hand resolution.
    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(PostRiver);

    // Trigger showdown logic (hand evaluation + pot distribution).
    // if (myEvents.onShowdown)
    //    myEvents.onShowdown(hand.getBoard(), hand.getRunningPlayersList());
}

void PostRiverState::exit(HandFsm& /*hand*/)
{
    // Nothing to do on exit; this is usually a terminal state.
}

std::unique_ptr<IHandState> PostRiverState::computeNextState(HandFsm& /*hand*/, const PlayerAction /*action*/)
{
    // PostRiver is terminal — no next state.
    return nullptr;
}

bool PostRiverState::isRoundComplete(const HandFsm& /*hand*/) const
{
    // Always true, as this is the end of the hand.
    return true;
}

bool PostRiverState::isActionAllowed(const HandFsm& /*hand*/, const PlayerAction /*action*/) const
{
    // No player actions allowed in PostRiver.
    return false;
}

void PostRiverState::promptPlayerAction(HandFsm& /*hand*/, player::PlayerFsm& /*player*/)
{
    // No actions to prompt in PostRiver.
}

void PostRiverState::logStateInfo(const HandFsm& /*hand*/) const
{
    // Could log showdown info, winners, etc.
}

} // namespace pkt::core