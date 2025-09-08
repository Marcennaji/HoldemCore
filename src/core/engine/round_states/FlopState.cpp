#include "FlopState.h"
#include "GameEvents.h"
#include "HandFsm.h"
#include "PostRiverState.h"
#include "TurnState.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/Helpers.h"
#include "core/player/PlayerFsm.h"
#include "core/services/GlobalServices.h"

namespace pkt::core
{
using namespace pkt::core::player;

FlopState::FlopState(const GameEvents& events) : myEvents(events)
{
}

void FlopState::enter(HandFsm& hand)
{
    GlobalServices::instance().logger().info("FlopState: Entering flop");
    // Reset betting amounts for new round
    hand.getBettingActions()->resetRoundHighestSet();

    for (auto& player : *hand.getRunningPlayersList())
    {
        player->setAction(*this, {player->getId(), ActionType::None});
    }
    // Deal flop cards
    // hand.getBoard()->dealFlop();

    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(Flop);
}

void FlopState::exit(HandFsm& hand)
{
    for (auto& player : *hand.getSeatsList())
    {
        player->getStatisticsUpdater()->updateFlopStatistics(player->getCurrentHandContext());
        player->getStatisticsUpdater()->updateFlopStatistics(player->getCurrentHandContext());
    }
    GlobalServices::instance().logger().info("FlopState: Exiting flop");
}

bool FlopState::isActionAllowed(const HandFsm& hand, const PlayerAction action) const
{
    return (validatePlayerAction(hand.getRunningPlayersList(), action, *hand.getBettingActions(), 0, Flop));
}

void FlopState::promptPlayerAction(HandFsm& hand, PlayerFsm& player)
{
    player.updateCurrentHandContext(Flop, hand);
    PlayerAction action = player.decideAction(player.getCurrentHandContext());

    hand.handlePlayerAction(action);
}

std::unique_ptr<IHandState> FlopState::computeNextState(HandFsm& hand, PlayerAction action)
{
    // If less than 2 players are still in hand (haven't folded), go directly to showdown
    if (hand.getPlayersInHandList()->size() < 2)
    {
        return std::make_unique<PostRiverState>(myEvents);
    }

    // If all remaining players are all-in (no one can act further), go directly to showdown
    if (hand.getRunningPlayersList()->empty() && hand.getPlayersInHandList()->size() >= 1)
    {
        return std::make_unique<PostRiverState>(myEvents);
    }

    // If round is complete and multiple players can still act, continue to Turn
    if (isRoundComplete(hand))
    {
        return std::make_unique<TurnState>(myEvents);
    }

    return nullptr;
}

void FlopState::logStateInfo(const HandFsm& /*hand*/) const
{
    // TODO: add logging (e.g. pot size, board cards, etc.)
}

} // namespace pkt::core