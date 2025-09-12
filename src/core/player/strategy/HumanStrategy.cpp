#include "HumanStrategy.h"
#include "CurrentHandContext.h"
#include "core/engine/Helpers.h"

namespace pkt::core::player
{

PlayerAction HumanStrategy::decideAction(const CurrentHandContext& ctx)
{
    // TODO: We need a way to get the HandFsm reference to calculate valid actions
    // For now, fire the event with empty valid actions
    std::vector<ActionType> validActions = {}; // Placeholder

    // Fire event to notify UI with player ID and valid actions
    if (myEvents.onAwaitingHumanInput)
    {
        myEvents.onAwaitingHumanInput(ctx.personalContext.id, validActions);
    }

    // Delegate to the input handler (which can now be UI-driven)
    return myInputHandler.requestAction(ctx);
}

} // namespace pkt::core::player