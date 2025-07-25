#include "PreflopState.h"

#include "FlopState.h"
#include "GameEvents.h"
#include "HandFsm.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/BotPlayer.h"
#include "core/player/Helpers.h"
#include "core/player/Player.h"

namespace pkt::core
{
using namespace pkt::core::player;

PreflopState::PreflopState(const GameEvents& events) : myEvents(events)
{
}

void PreflopState::enter(HandFsm& hand)
{
    // determine dealer, SB, BB
    // assignButtons();

    // setBlinds();

    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(GameStatePreflop);
}

void PreflopState::exit(HandFsm& /*hand*/)
{
    // No exit action needed for Preflop
}

bool PreflopState::canProcessAction(const HandFsm& hand, const PlayerAction action) const
{
    auto player = getPlayerFsmById(hand.getRunningPlayersList(), action.playerId);
    if (!player)
        return false;

    const int cash = player->getLegacyPlayer()->getCash();
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

    return false;
}

void PreflopState::handlePlayerAction(HandFsm& hand, Player& player)
{
    if (!player.isBot())
        return;

    auto& bot = static_cast<BotPlayer&>(player);
    const PlayerAction action = bot.decidePreflopActionFsm();
    hand.handlePlayerAction(action);
}

std::unique_ptr<IHandState> PreflopState::processAction(HandFsm& hand, PlayerAction action)
{
    if (isRoundComplete(hand))
    {
        exit(hand);
        return std::make_unique<FlopState>(myEvents);
    }

    return nullptr;
}

bool PreflopState::isRoundComplete(const HandFsm& hand) const
{
    int highestSet = -1;

    if (hand.getRunningPlayersList()->size() <= 1)
        return true;

    for (auto itC = hand.getRunningPlayersList()->begin(); itC != hand.getRunningPlayersList()->end(); ++itC)
    {
        if (highestSet == -1)
            highestSet = (*itC)->getLegacyPlayer()->getSet();
        else
        {
            if (highestSet != (*itC)->getLegacyPlayer()->getSet())
            {
                return false;
            }
        }
    }

    return true;
}

void PreflopState::logStateInfo(const HandFsm& /*hand*/) const
{
    // Optional: Add logging when debugging
}

} // namespace pkt::core
