#include "ActionApplier.h"
#include "core/engine/hand/Hand.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/Helpers.h"
#include "core/player/Player.h"
#include "core/player/PlayerListUtils.h"

namespace pkt::core
{

using namespace pkt::core::player;

void ActionApplier::apply(Hand& hand, const PlayerAction& action)
{
    auto player = getPlayerById(hand.getActingPlayersList(), action.playerId);
    if (!player)
        return;

    int currentHighest = hand.getBettingActions()->getRoundHighestSet();
    int playerBet = player->getCurrentHandActions().getRoundTotalBetAmount(hand.getStateManager()->getGameState());

    // Create a copy for storing in action history with correct increment amounts
    PlayerAction actionForHistory = action;

    switch (action.type)
    {
    case ActionType::Fold:
        // No amount change needed
        break;
    case ActionType::Call:
        applyCallAction(hand, *player, actionForHistory, currentHighest, playerBet);
        break;
    case ActionType::Raise:
        applyRaiseAction(hand, *player, actionForHistory, action, playerBet);
        break;
    case ActionType::Bet:
        applyBetAction(hand, *player, action);
        break;
    case ActionType::Check:
        // No amount change needed
        break;
    case ActionType::Allin:
        applyAllinAction(hand, *player, actionForHistory, currentHighest);
        break;
    case ActionType::None:
    default:
        break;
    }

    finalizeAction(hand, *player, actionForHistory);
}

void ActionApplier::applyCallAction(Hand& hand, player::Player& player, PlayerAction& actionForHistory,
                                    int currentHighest, int playerBet)
{
    int amountToCall = currentHighest - playerBet;
    if (player.getCash() < amountToCall)
    {
        amountToCall = player.getCash();
    }

    // If this call uses all remaining cash, treat it as an all-in
    if (amountToCall == player.getCash())
    {
        // Mark as all-in for history/display purposes, but do not set cash to 0 here;
        // updateBetAndPot will subtract the amount once. Setting cash to 0 beforehand
        // would cause a double-debit (0 - amount) leading to negative chips and an
        // incorrect extra onPlayerChipsUpdated event.
        actionForHistory.type = ActionType::Allin;
    }

    actionForHistory.amount = amountToCall; // store the actual call amount
    updateBetAndPot(hand, player, amountToCall);
}

void ActionApplier::applyRaiseAction(Hand& hand, player::Player& player, PlayerAction& actionForHistory,
                                     const PlayerAction& originalAction, int playerBet)
{
    int raiseIncrement = originalAction.amount - playerBet;
    assert(raiseIncrement > 0);
    actionForHistory.amount = raiseIncrement; // store only the increment in history
    updateBetAndPot(hand, player, raiseIncrement);
    hand.getBettingActions()->updateRoundHighestSet(originalAction.amount);
    setLastRaiserForCurrentRound(hand, player.getId());
}

void ActionApplier::applyBetAction(Hand& hand, player::Player& player, const PlayerAction& action)
{
    // For bet, the amount is already the increment
    updateBetAndPot(hand, player, action.amount);
    hand.getBettingActions()->updateRoundHighestSet(action.amount);
    setLastRaiserForCurrentRound(hand, player.getId());
}

void ActionApplier::applyAllinAction(Hand& hand, player::Player& player, PlayerAction& actionForHistory,
                                     int currentHighest)
{
    int allinIncrement = player.getCash();    // The increment is all remaining cash
    actionForHistory.amount = allinIncrement; // store only the increment in history

    updateBetAndPot(hand, player, allinIncrement);
    // No need to setCash(0) explicitly here; after subtracting all remaining cash,
    // player.getCash() is already 0 and updateBetAndPot emitted the update event.

    if (allinIncrement > currentHighest)
    {
        hand.getBettingActions()->updateRoundHighestSet(allinIncrement);
        setLastRaiserForCurrentRound(hand, player.getId());
    }
}

void ActionApplier::updateBetAndPot(Hand& hand, player::Player& player, int amount)
{
    player.addBetAmount(amount);
    hand.fireOnPotUpdated();
}

void ActionApplier::setLastRaiserForCurrentRound(Hand& hand, unsigned playerId)
{
    switch (hand.getStateManager()->getGameState())
    {
    case GameState::Preflop:
        hand.getBettingActions()->getPreflop().setLastRaiserId(playerId);
        break;
    case GameState::Flop:
        hand.getBettingActions()->getFlop().setLastRaiserId(playerId);
        break;
    case GameState::Turn:
        hand.getBettingActions()->getTurn().setLastRaiserId(playerId);
        break;
    case GameState::River:
        hand.getBettingActions()->getRiver().setLastRaiserId(playerId);
        break;
    default:
        break;
    }
}

void ActionApplier::finalizeAction(Hand& hand, player::Player& player, const PlayerAction& actionForHistory)
{
    // Record action in hand-level chronological history
    hand.getBettingActions()->recordPlayerAction(hand.getStateManager()->getGameState(), actionForHistory);

    player.processAction(actionForHistory, hand);

    updateActingPlayersList(hand.getActingPlayersListMutable());

    if (hand.getEvents().onPlayerActed)
    {
        hand.getEvents().onPlayerActed(actionForHistory);
    }
}

} // namespace pkt::core
