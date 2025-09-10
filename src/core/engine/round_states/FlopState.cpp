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
    GlobalServices::instance().logger().info("Flop");
    // Reset betting amounts for new round
    hand.getBettingActions()->resetRoundHighestSet();

    for (auto& player : *hand.getActingPlayersList())
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
        player->updateCurrentHandContext(GameState::Flop, hand);
        player->getStatisticsUpdater()->updateFlopStatistics(player->getCurrentHandContext());
    }
}

bool FlopState::isActionAllowed(const HandFsm& hand, const PlayerAction action) const
{
    return (validatePlayerAction(hand.getActingPlayersList(), action, *hand.getBettingActions(), 0, Flop));
}

void FlopState::promptPlayerAction(HandFsm& hand, PlayerFsm& player)
{
    player.updateCurrentHandContext(Flop, hand);
    PlayerAction action = player.decideAction(player.getCurrentHandContext());

    hand.handlePlayerAction(action);
}

std::unique_ptr<IHandState> FlopState::computeNextState(HandFsm& hand)
{
    return computeBettingRoundNextState(hand, myEvents, Flop);
}

std::shared_ptr<player::PlayerFsm> FlopState::getNextPlayerToAct(const HandFsm& hand) const
{
    return getFirstPlayerToActPostFlop(hand);
}

std::shared_ptr<player::PlayerFsm> FlopState::getFirstPlayerToActInRound(const HandFsm& hand) const
{
    return getNextPlayerToAct(hand);
}

bool FlopState::isRoundComplete(const HandFsm& hand) const
{
    return pkt::core::player::isRoundComplete(const_cast<HandFsm&>(hand));
}

void FlopState::logStateInfo(const HandFsm& /*hand*/) const
{
    // TODO: add logging (e.g. pot size, board cards, etc.)
}

} // namespace pkt::core