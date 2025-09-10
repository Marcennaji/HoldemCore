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
    GlobalServices::instance().logger().info("River");

    for (auto& player : *hand.getActingPlayersList())
    {
        player->setAction(*this, {player->getId(), ActionType::None});
    }

    // Reset betting amounts for new round
    hand.getBettingActions()->resetRoundHighestSet();

    // Deal river card
    // hand.getBoard()->dealRiver();

    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(River);
}

void RiverState::exit(HandFsm& hand)
{
    for (auto& player : *hand.getSeatsList())
    {
        player->updateCurrentHandContext(GameState::River, hand);
        player->getStatisticsUpdater()->updateRiverStatistics(player->getCurrentHandContext());
    }
}

bool RiverState::isActionAllowed(const HandFsm& hand, const PlayerAction action) const
{
    return (validatePlayerAction(hand.getActingPlayersList(), action, *hand.getBettingActions(), 0, River));
}

void RiverState::promptPlayerAction(HandFsm& hand, PlayerFsm& player)
{
    player.updateCurrentHandContext(River, hand);
    PlayerAction action = player.decideAction(player.getCurrentHandContext());

    hand.handlePlayerAction(action);
}

std::unique_ptr<IHandState> RiverState::computeNextState(HandFsm& hand)
{
    return computeBettingRoundNextState(hand, myEvents, River);
}

std::shared_ptr<player::PlayerFsm> RiverState::getNextPlayerToAct(const HandFsm& hand) const
{
    return getFirstPlayerToActPostFlop(hand);
}

std::shared_ptr<player::PlayerFsm> RiverState::getFirstPlayerToActInRound(const HandFsm& hand) const
{
    return getNextPlayerToAct(hand);
}
bool RiverState::isRoundComplete(const HandFsm& hand) const
{
    return pkt::core::player::isRoundComplete(const_cast<HandFsm&>(hand));
}

void RiverState::logStateInfo(const HandFsm& /*hand*/) const
{
    // TODO: add logging (e.g. pot size, board cards, etc.)
}

} // namespace pkt::core