#include "HumanStrategy.h"
#include "CurrentHandContext.h"
#include "core/engine/utils/Helpers.h"

namespace pkt::core::player
{

PlayerAction HumanStrategy::decideAction(const CurrentHandContext& ctx)
{
    // Get valid actions from the context (calculated during updateCurrentHandContext)
    const std::vector<ActionType>& validActions = ctx.commonContext.validActions;

    // Fire event to notify UI with player ID and valid actions
    if (myEvents.onAwaitingHumanInput)
    {
        myEvents.onAwaitingHumanInput(ctx.personalContext.id, validActions);
    }

    // Delegate to the input handler (which can now be UI-driven)
    return myInputHandler.requestAction(ctx);
}

} // namespace pkt::core::player