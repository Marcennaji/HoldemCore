#include "PostRiverState.h"
#include "GameEvents.h"
#include "HandFsm.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/Helpers.h"
#include "core/player/PlayerFsm.h"
#include "core/services/GlobalServices.h"

namespace pkt::core
{
using namespace pkt::core::player;

PostRiverState::PostRiverState(const GameEvents& events) : myEvents(events)
{
}

void PostRiverState::enter(HandFsm& hand)
{
    GlobalServices::instance().logger().info("PostRiverState: Entering post-river");

    for (auto& player : *hand.getRunningPlayersList())
    {
        player->setAction(*this, {player->getId(), ActionType::None});
    }

    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(PostRiver);
}

void PostRiverState::exit(HandFsm& /*hand*/)
{
    GlobalServices::instance().logger().info("PostRiverState: Exiting post-river");
}

std::unique_ptr<IHandState> PostRiverState::computeNextState(HandFsm& /*hand*/, const PlayerAction /*action*/)
{
    // PostRiver is terminal — no next state.
    return nullptr;
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