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
    m_actionPromise = std::make_unique<std::promise<PlayerAction>>();
    std::future<PlayerAction> actionFuture = m_actionPromise->get_future();

    // Register this strategy as the one waiting for input
    if (m_events.onHumanStrategyWaiting)
    {
        m_events.onHumanStrategyWaiting(this);
    }

    // Fire event to notify UI with player ID and valid actions
    if (m_events.onAwaitingHumanInput)
    {
        m_events.onAwaitingHumanInput(ctx.personalContext.id, validActions);
    }

    // Wait for UI to provide the action via setPlayerAction()
    // Use timeout-based waiting with UI event processing to keep interface responsive
    while (actionFuture.wait_for(std::chrono::milliseconds(10)) == std::future_status::timeout)
    {
        // Process UI framework events to keep interface responsive during wait
        if (m_events.onProcessEvents)
        {
            m_events.onProcessEvents();
        }
    }
    
    PlayerAction action = actionFuture.get();
    
    // Ensure the player ID is set correctly
    action.playerId = ctx.personalContext.id;
    return action;
}

void HumanStrategy::setPlayerAction(const PlayerAction& action)
{
    if (m_actionPromise)
    {
        m_actionPromise->set_value(action);
        m_actionPromise.reset(); // Clear the promise
    }
}

} // namespace pkt::core::player