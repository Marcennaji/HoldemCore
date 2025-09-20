#include "PostRiverState.h"
#include "GameEvents.h"
#include "Hand.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/Helpers.h"
#include "core/player/Player.h"
#include "core/services/ServiceContainer.h"

namespace pkt::core
{
using namespace pkt::core::player;

PostRiverState::PostRiverState(const GameEvents& events) : myEvents(events)
{
}

PostRiverState::PostRiverState(const GameEvents& events, std::shared_ptr<pkt::core::ServiceContainer> services)
    : myEvents(events), myServices(std::move(services))
{
}

void PostRiverState::ensureServicesInitialized() const
{
    if (!myServices)
    {
        myServices = std::make_shared<pkt::core::AppServiceContainer>();
    }
}

void PostRiverState::enter(Hand& hand)
{
    ensureServicesInitialized();
    myServices->logger().info("Post-River");

    for (auto& player : *hand.getActingPlayersList())
    {
        player->setAction(*this, {player->getId(), ActionType::None});
    }

    hand.getBoard().distributePot(hand);

    hand.getBoard().determinePlayerNeedToShowCards();

    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(PostRiver);

    logStateInfo(hand);
}

void PostRiverState::exit(Hand& hand)
{
    // Clear betting data after pot distribution is complete
    for (auto& player : *hand.getSeatsList())
    {
        player->resetCurrentHandActions();
    }
}

std::unique_ptr<IHandState> PostRiverState::computeNextState(Hand& /*hand*/)
{
    // PostRiver is terminal — no next state.
    return nullptr;
}

bool PostRiverState::isActionAllowed(const Hand& /*hand*/, const PlayerAction /*action*/) const
{
    // No player actions allowed in PostRiver.
    return false;
}

void PostRiverState::promptPlayerAction(Hand& /*hand*/, player::Player& /*player*/)
{
    // No player actions in PostRiver.
}

std::shared_ptr<player::Player> PostRiverState::getNextPlayerToAct(const Hand& /*hand*/) const
{
    return nullptr;
}

std::shared_ptr<player::Player> PostRiverState::getFirstPlayerToActInRound(const Hand& /*hand*/) const
{
    return nullptr;
}

bool PostRiverState::isRoundComplete(const Hand& /*hand*/) const
{
    return true;
}

} // namespace pkt::core