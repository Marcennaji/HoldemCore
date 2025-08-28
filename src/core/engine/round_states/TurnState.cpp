#include "TurnState.h"
#include "GameEvents.h"
#include "HandFsm.h"
#include "PostRiverState.h"
#include "RiverState.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/Helpers.h"
#include "core/player/PlayerFsm.h"

namespace pkt::core
{
using namespace pkt::core::player;

TurnState::TurnState(const GameEvents& events) : myEvents(events)
{
}

void TurnState::enter(HandFsm& hand)
{
    // Reset betting amounts for new round
    hand.getBettingState()->resetHighestSet();

    // Deal turn card
    // hand.getBoard()->dealTurn();

    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(Turn);
}

void TurnState::exit(HandFsm& /*hand*/)
{
    // No special exit logic for Turn
}

bool TurnState::isActionAllowed(const HandFsm& hand, const PlayerAction action) const
{
    auto player = getPlayerFsmById(hand.getRunningPlayersList(), action.playerId);
    if (!player)
        return false;

    const int cash = player->getCash();
    const int amountToCall = hand.getBettingState()->getHighestSet() - player->getTotalBetAmount();

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

void TurnState::promptPlayerAction(HandFsm& hand, PlayerFsm& player)
{
    player.updateCurrentHandContext(Turn, hand);
    const PlayerAction action = player.decideAction(player.getCurrentHandContext());

    hand.handlePlayerAction(action);
}

std::unique_ptr<IHandState> TurnState::computeNextState(HandFsm& hand, PlayerAction action)
{
    if (hand.getRunningPlayersList()->size() == 1)
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

bool TurnState::isRoundComplete(const HandFsm& hand) const
{
    if (hand.getRunningPlayersList()->size() <= 1)
        return true;

    for (auto itC = hand.getRunningPlayersList()->begin(); itC != hand.getRunningPlayersList()->end(); ++itC)
    {
        if ((*itC)->getTotalBetAmount() != hand.getBettingState()->getHighestSet())
        {
            return false;
        }
    }

    return true;
}

void TurnState::logStateInfo(const HandFsm& /*hand*/) const
{
    // TODO: add logging (e.g. pot size, board cards, etc.)
}

} // namespace pkt::core