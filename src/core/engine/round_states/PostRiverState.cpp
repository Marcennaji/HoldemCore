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
    GlobalServices::instance().logger().info("Post-River");

    for (auto& player : *hand.getActingPlayersList())
    {
        player->setAction(*this, {player->getId(), ActionType::None});
    }

    hand.getBoard().distributePot(hand);

    hand.getBoard().determinePlayerNeedToShowCards();

    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(PostRiver);
}

void PostRiverState::exit(HandFsm& hand)
{
    // Clear betting data after pot distribution is complete
    for (auto& player : *hand.getSeatsList())
    {
        player->resetCurrentHandActions();
    }
}

std::unique_ptr<IHandState> PostRiverState::computeNextState(HandFsm& /*hand*/)
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
    // No player actions in PostRiver.
}

std::shared_ptr<player::PlayerFsm> PostRiverState::getNextPlayerToAct(const HandFsm& /*hand*/) const
{
    return nullptr;
}

std::shared_ptr<player::PlayerFsm> PostRiverState::getFirstPlayerToActInRound(const HandFsm& /*hand*/) const
{
    return nullptr;
}

bool PostRiverState::isRoundComplete(const HandFsm& /*hand*/) const
{
    return true;
}

void PostRiverState::logStateInfo(const HandFsm& /*hand*/) const
{
    // todo
}

} // namespace pkt::core