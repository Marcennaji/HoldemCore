#include "TurnState.h"
#include "GameEvents.h"
#include "HandFsm.h"
#include "PostRiverState.h"
#include "RiverState.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/Helpers.h"
#include "core/player/PlayerFsm.h"
#include "core/services/GlobalServices.h"

namespace pkt::core
{
using namespace pkt::core::player;

TurnState::TurnState(const GameEvents& events) : myEvents(events)
{
}

void TurnState::enter(HandFsm& hand)
{
    GlobalServices::instance().logger().info("TurnState: Entering turn");

    for (auto& player : *hand.getRunningPlayersList())
    {
        player->setAction(*this, {player->getId(), ActionType::None});
    }
    // Reset betting amounts for new round
    hand.getBettingActions()->resetRoundHighestSet();

    // Deal turn card
    // hand.getBoard()->dealTurn();

    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(Turn);
}

void TurnState::exit(HandFsm& hand)
{
    for (auto& player : *hand.getSeatsList())
    {
        player->getStatisticsUpdater()->updateTurnStatistics(player->getCurrentHandContext());
    }
    GlobalServices::instance().logger().info("TurnState: Exiting turn");
}

bool TurnState::isActionAllowed(const HandFsm& hand, const PlayerAction action) const
{
    return (validatePlayerAction(hand.getRunningPlayersList(), action, *hand.getBettingActions(), 0, Turn));
}

void TurnState::promptPlayerAction(HandFsm& hand, PlayerFsm& player)
{
    player.updateCurrentHandContext(Turn, hand);
    PlayerAction action = player.decideAction(player.getCurrentHandContext());

    hand.handlePlayerAction(action);
}

std::unique_ptr<IHandState> TurnState::computeNextState(HandFsm& hand, PlayerAction action)
{
    // If less than 2 players are still in hand (haven't folded), go directly to showdown
    if (hand.getPlayersInHandList()->size() < 2)
    {
        exit(hand);
        return std::make_unique<PostRiverState>(myEvents);
    }

    // If all remaining players are all-in (no one can act further), go directly to showdown
    if (hand.getRunningPlayersList()->empty() && hand.getPlayersInHandList()->size() >= 1)
    {
        exit(hand);
        return std::make_unique<PostRiverState>(myEvents);
    }

    if (isRoundComplete(hand))
    {
        exit(hand);
        return std::make_unique<RiverState>(myEvents);
    }

    return nullptr;
}

void TurnState::logStateInfo(const HandFsm& /*hand*/) const
{
    // TODO: add logging (e.g. pot size, board cards, etc.)
}

} // namespace pkt::core