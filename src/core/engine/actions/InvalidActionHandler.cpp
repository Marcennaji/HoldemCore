#include "InvalidActionHandler.h"
#include "GameEvents.h"
#include "core/services/ServiceContainer.h"

namespace pkt::core
{

InvalidActionHandler::InvalidActionHandler(const GameEvents& events, ErrorMessageProvider errorProvider,
                                           AutoFoldCallback autoFoldCallback)
    : myEvents(events), myErrorMessageProvider(std::move(errorProvider)),
      myAutoFoldCallback(std::move(autoFoldCallback))
{
}

InvalidActionHandler::InvalidActionHandler(const GameEvents& events, ErrorMessageProvider errorProvider,
                                           AutoFoldCallback autoFoldCallback,
                                           std::shared_ptr<pkt::core::ServiceContainer> services)
    : myEvents(events), myErrorMessageProvider(std::move(errorProvider)),
      myAutoFoldCallback(std::move(autoFoldCallback)), myServices(std::move(services))
{
}

void InvalidActionHandler::ensureServicesInitialized() const
{
    if (!myServices)
    {
        myServices = std::make_shared<pkt::core::AppServiceContainer>();
    }
}

void InvalidActionHandler::handleInvalidAction(const PlayerAction& action)
{
    ensureServicesInitialized();

    // Track invalid action attempts
    myInvalidActionCounts[action.playerId]++;

    // Fire event to notify UI of invalid action
    if (myEvents.onInvalidPlayerAction)
    {
        std::string reason = myErrorMessageProvider(action);
        myEvents.onInvalidPlayerAction(action.playerId, action, reason);
    }

    myServices->logger().error("Invalid action from player " + std::to_string(action.playerId) + " (attempt " +
                               std::to_string(myInvalidActionCounts[action.playerId]) +
                               "): " + myErrorMessageProvider(action));

    // Check if player should be auto-folded due to repeated invalid actions
    if (shouldAutoFoldPlayer(action.playerId))
    {
        executeAutoFold(action.playerId);
    }
}

void InvalidActionHandler::resetInvalidActionCount(unsigned playerId)
{
    myInvalidActionCounts[playerId] = 0;
}

bool InvalidActionHandler::shouldAutoFoldPlayer(unsigned playerId) const
{
    auto it = myInvalidActionCounts.find(playerId);
    return (it != myInvalidActionCounts.end() && it->second >= MAX_INVALID_ACTIONS);
}

int InvalidActionHandler::getInvalidActionCount(unsigned playerId) const
{
    auto it = myInvalidActionCounts.find(playerId);
    return (it != myInvalidActionCounts.end()) ? it->second : 0;
}

void InvalidActionHandler::executeAutoFold(unsigned playerId)
{
    ensureServicesInitialized();

    myServices->logger().error("Player " + std::to_string(playerId) +
                               " exceeded maximum invalid actions, auto-folding");

    if (myEvents.onEngineError)
    {
        myEvents.onEngineError("Player " + std::to_string(playerId) + " auto-folded due to repeated invalid actions");
    }

    // Reset counter and trigger auto-fold callback
    resetInvalidActionCount(playerId);
    myAutoFoldCallback(playerId);
}

} // namespace pkt::core