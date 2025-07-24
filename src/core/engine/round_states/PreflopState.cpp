#include "PreflopState.h"

#include "FlopState.h"
#include "GameEvents.h"
#include "Hand.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/BotPlayer.h"
#include "core/player/Helpers.h"
#include "core/player/Player.h"

namespace pkt::core
{
using namespace pkt::core::player;

PreflopState::PreflopState(GameEvents& events) : myEvents(events)
{
}

void PreflopState::enter(IHand& hand)
{
    hand.postBlindsFsm();
    hand.prepareBettingRoundFsm();

    const auto& players = *hand.getRunningPlayersList();
    if (!players.empty())
    {
        hand.setCurrentPlayerIdFsm(hand.getRunningPlayersList()->front()->getId());
    }
    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(GameStatePreflop);
}

void PreflopState::exit(IHand& /*hand*/)
{
    // No exit action needed for Preflop
}

bool PreflopState::canProcessAction(const IHand& hand, const PlayerAction action) const
{
    auto player = getPlayerById(hand.getRunningPlayersList(), action.playerId);
    if (!player)
        return false;

    // If you want to restrict to "only current player can act", uncomment:
    // if (hand.getCurrentPlayerId() != action.playerId)
    //     return false;

    const int cash = player->getCash();
    const int callAmount = 10; // Replace with: hand.amountToCall(player->id());

    switch (action.type)
    {
    case ActionType::Fold:
        return true;

    case ActionType::Check:
        return callAmount == 0;

    case ActionType::Call:
        return callAmount > 0 && cash >= callAmount;

    case ActionType::Bet:
        return callAmount == 0 && action.amount > 0 && action.amount <= cash;

    case ActionType::Raise:
        // You might want to enforce minimum raise rules here
        // return callAmount > 0 && action.amount >= hand.minRaiseAmount() && action.amount <= cash;
        return callAmount > 0 && action.amount <= cash;

    case ActionType::Allin:
        return cash > 0;

    default:
        return false;
    }
}

void PreflopState::handlePlayerAction(IHand& hand, Player& player)
{
    if (!player.isBot())
        return;

    auto& bot = static_cast<BotPlayer&>(player);
    const PlayerAction action = bot.decidePreflopActionFsm();
    hand.applyActionFsm(action);
}

std::unique_ptr<IHandState> PreflopState::processAction(IHand& hand, PlayerAction action)
{
    hand.applyActionFsm(action);

    if (isRoundComplete(hand))
    {
        exit(hand);
        return std::make_unique<FlopState>(myEvents);
    }

    hand.advanceToNextPlayerFsm();
    return nullptr;
}

bool PreflopState::isRoundComplete(const IHand& hand) const
{
    int highestSet = -1;

    if (hand.getRunningPlayersList()->size() <= 1)
        return true;

    for (auto itC = hand.getRunningPlayersList()->begin(); itC != hand.getRunningPlayersList()->end(); ++itC)
    {
        if (highestSet == -1)
            highestSet = (*itC)->getSet();
        else
        {
            if (highestSet != (*itC)->getSet())
            {
                return false;
            }
        }
    }

    return true;
}

void PreflopState::logStateInfo(const IHand& /*hand*/) const
{
    // Optional: Add logging when debugging
}

} // namespace pkt::core
