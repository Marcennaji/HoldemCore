#include "RiverState.h"
#include "GameEvents.h"
#include "HandFsm.h"
#include "PostRiverState.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/Helpers.h"
#include "core/player/PlayerFsm.h"

namespace pkt::core
{
using namespace pkt::core::player;

RiverState::RiverState(const GameEvents& events) : myEvents(events)
{
}

void RiverState::enter(HandFsm& hand)
{
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
}

bool RiverState::isActionAllowed(const HandFsm& hand, const PlayerAction action) const
{
    auto player = getPlayerFsmById(hand.getRunningPlayersList(), action.playerId);
    if (!player)
        return false;

    const int cash = player->getCash();
    const int amountToCall = hand.getBettingActions()->getHighestSet() - player->getTotalBetAmount();

    switch (action.type)
    {
    case ActionType::Fold:
        return true;

    case ActionType::Check:
        return amountToCall == 0 && action.amount == 0;

    case ActionType::Call:
        return action.amount == amountToCall && cash >= action.amount;

    case ActionType::Bet:
        return amountToCall == 0 && action.amount > 0 && action.amount <= cash;

    case ActionType::Raise:
        return amountToCall > 0 && action.amount > amountToCall && action.amount <= cash;

    case ActionType::Allin:
        return cash > 0;

    default:
        return false;
    }
}

void RiverState::promptPlayerAction(HandFsm& hand, PlayerFsm& player)
{
    player.updateCurrentHandContext(River, hand);
    const PlayerAction action = player.decideAction(player.getCurrentHandContext());

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

bool RiverState::isRoundComplete(const HandFsm& hand) const
{
    if (hand.getRunningPlayersList()->size() <= 1)
        return true;

    for (auto itC = hand.getRunningPlayersList()->begin(); itC != hand.getRunningPlayersList()->end(); ++itC)
    {
        if ((*itC)->getTotalBetAmount() != hand.getBettingActions()->getHighestSet())
        {
            return false;
        }
    }

    return true;
}

void RiverState::logStateInfo(const HandFsm& /*hand*/) const
{
    // TODO: add logging (e.g. pot size, board cards, etc.)
}

} // namespace pkt::core