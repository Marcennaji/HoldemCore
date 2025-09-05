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
    // Reset betting amounts for new round
    hand.getBettingActions()->resetHighestSet();

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
}

bool FlopState::isActionAllowed(const HandFsm& hand, const PlayerAction action) const
{
    auto player = getPlayerFsmById(hand.getRunningPlayersList(), action.playerId);
    if (!player)
    {
        GlobalServices::instance().logger().error("FlopState: Player " + std::to_string(action.playerId) +
                                                  " not found");
        return false;
    }
    if (validatePlayerAction(*player, action, *hand.getBettingActions(), 0))
    {
        return true;
    }
    GlobalServices::instance().logger().error(
        "FlopState: Invalid action for player " + std::to_string(action.playerId) + " : " +
        playerActionToString(action.type) + " with amount = " + std::to_string(action.amount));
    return false;
}

void FlopState::promptPlayerAction(HandFsm& hand, PlayerFsm& player)
{
    player.updateCurrentHandContext(Flop, hand);
    const PlayerAction action = player.decideAction(player.getCurrentHandContext());

    hand.handlePlayerAction(action);
}

std::unique_ptr<IHandState> FlopState::computeNextState(HandFsm& hand, PlayerAction action)
{
    if (hand.getRunningPlayersList()->size() == 1)
    {
        return std::make_unique<PostRiverState>(myEvents);
    }
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