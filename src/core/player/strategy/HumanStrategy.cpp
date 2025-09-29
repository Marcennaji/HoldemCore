#include "HumanStrategy.h"
#include "CurrentHandContext.h"
#include "core/engine/utils/Helpers.h"
#include <future>
#include <chrono>

namespace pkt::core::player
{

pkt::core::PlayerAction HumanStrategy::decideAction(const CurrentHandContext& ctx)
{
    // Get valid actions from the context
    const std::vector<ActionType>& validActions = ctx.commonContext.validActions;

    // set up a promise/future pair for asynchronous communication of a PlayerAction value
    myActionPromise = std::make_unique<std::promise<PlayerAction>>();
    std::future<PlayerAction> actionFuture = myActionPromise->get_future();

    // Register this strategy as the one waiting for input
    if (myEvents.onHumanStrategyWaiting)
    {
        myEvents.onHumanStrategyWaiting(this);
    }

    // Fire event to notify UI with player ID and valid actions
    if (myEvents.onAwaitingHumanInput)
    {
        myEvents.onAwaitingHumanInput(ctx.personalContext.id, validActions);
    }

    // Wait for UI to provide the action via setPlayerAction()
    // Use timeout-based waiting with UI event processing to keep interface responsive
    while (actionFuture.wait_for(std::chrono::milliseconds(10)) == std::future_status::timeout)
    {
        // Process UI framework events to keep interface responsive during wait
        if (myEvents.onProcessEvents)
        {
            myEvents.onProcessEvents();
        }
    }
    
    PlayerAction action = actionFuture.get();
    
    // Ensure the player ID is set correctly
    action.playerId = ctx.personalContext.id;
    return action;
}

void HumanStrategy::setPlayerAction(const PlayerAction& action)
{
    if (myActionPromise)
    {
        myActionPromise->set_value(action);
        myActionPromise.reset(); // Clear the promise
    }
}

} // namespace pkt::core::player