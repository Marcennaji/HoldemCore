#include "RiverState.h"
#include "GameEvents.h"
#include "HandFsm.h"
#include "PostRiverState.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/Helpers.h"
#include "core/player/PlayerFsm.h"
#include "core/services/GlobalServices.h"

namespace pkt::core
{
using namespace pkt::core::player;

RiverState::RiverState(const GameEvents& events) : myEvents(events)
{
}

void RiverState::enter(HandFsm& hand)
{
    GlobalServices::instance().logger().info("RiverState: Entering river");
    // Reset betting amounts for new round
    hand.getBettingActions()->resetHighestSet();

    // Deal river card
    // hand.getBoard()->dealRiver();

    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(River);
}

void RiverState::exit(HandFsm& hand)
{
    for (auto& player : *hand.getSeatsList())
    {
        player->getStatisticsUpdater()->updateRiverStatistics(player->getCurrentHandContext());
    }
    GlobalServices::instance().logger().info("RiverState: Exiting river");
}

bool RiverState::isActionAllowed(const HandFsm& hand, const PlayerAction action) const
{
    return (validatePlayerAction(hand.getRunningPlayersList(), action, *hand.getBettingActions(), 0, River));
}

void RiverState::promptPlayerAction(HandFsm& hand, PlayerFsm& player)
{
    player.updateCurrentHandContext(River, hand);
    PlayerAction action = player.decideAction(player.getCurrentHandContext());

    hand.handlePlayerAction(action);
}

std::unique_ptr<IHandState> RiverState::computeNextState(HandFsm& hand, PlayerAction action)
{

    if (isRoundComplete(hand))
    {
        exit(hand);
        return std::make_unique<PostRiverState>(myEvents);
    }

    return nullptr;
}

void RiverState::logStateInfo(const HandFsm& /*hand*/) const
{
    // TODO: add logging (e.g. pot size, board cards, etc.)
}

} // namespace pkt::core