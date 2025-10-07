#include "ActionApplier.h"
#include "core/engine/hand/Hand.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/Helpers.h"
#include "core/player/Player.h"
#include "core/player/PlayerListUtils.h"

namespace pkt::core
{

using namespace pkt::core::player;

void ActionApplier::apply(Hand& hand, const PlayerAction& action, pkt::core::Logger& logger)
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

    finalizeAction(hand, *player, actionForHistory, logger);
}

void ActionApplier::applyCallAction(Hand& hand, player::Player& player, PlayerAction& actionForHistory,
                                    int currentHighest, int playerBet)
{
    // Derive an effective highest bet to call. In some edge cases (e.g., an all-in shove),
    // the round highest may not have been updated yet OR the game may have advanced to
    // PostRiver to fast-forward streets. In those cases, compute from both round totals and
    // hand totals and use the larger required amount.
    int derivedHighestRound = currentHighest;
    const auto state = hand.getStateManager()->getGameState();

    int playerRoundBet = playerBet;
    int playerHandTotal = player.getCurrentHandActions().getHandTotalBetAmount();

    int derivedHighestHand = 0;

    for (const auto& p : *hand.getSeatsList())
    {
        // Highest absolute committed this round (for regular call sizing in-round)
        derivedHighestRound = std::max(derivedHighestRound, p->getCurrentHandActions().getRoundTotalBetAmount(state));
        // Highest absolute committed to the hand (covers fast-forwarded all-in scenarios)
        derivedHighestHand = std::max(derivedHighestHand, p->getCurrentHandActions().getHandTotalBetAmount());
    }

    int amountToCallRound = std::max(0, derivedHighestRound - playerRoundBet);
    int amountToCallHand = std::max(0, derivedHighestHand - playerHandTotal);

    // Prefer the larger of the two so we never undercall in fast-forwarded all-in cases.
    int amountToCall = std::max(amountToCallRound, amountToCallHand);

    // Clamp to player's available cash (call can be an all-in for the caller)
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
    // The increment equals all remaining cash; the absolute "to" amount is previous bet + increment.
    const auto state = hand.getStateManager()->getGameState();
    int playerRoundBetBefore = player.getCurrentHandActions().getRoundTotalBetAmount(state);
    int allinIncrement = player.getCash();    // The increment is all remaining cash
    actionForHistory.amount = allinIncrement; // store only the increment in history

    // Compute player's absolute total committed for this round after the shove (pre-history update)
    int playerBetAfter = playerRoundBetBefore + allinIncrement;

    // Apply the increment to the player's bet and pot
    updateBetAndPot(hand, player, allinIncrement);
    // No need to setCash(0) explicitly here; after subtracting all remaining cash,
    // player.getCash() is already 0 and updateBetAndPot emitted the update event.

    // If this all-in increases the absolute highest "to" amount, update it accordingly
    if (playerBetAfter > currentHighest)
    {
        hand.getBettingActions()->updateRoundHighestSet(playerBetAfter);
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
    auto player = getPlayerById(hand.getSeatsList(), playerId);
    if (!player)
    {
        return; // Player not found, ignore
    }

    switch (hand.getStateManager()->getGameState())
    {
    case GameState::Preflop:
        hand.getBettingActions()->getPreflop().setLastRaiser(player);
        break;
    case GameState::Flop:
        hand.getBettingActions()->getFlop().setLastRaiser(player);
        break;
    case GameState::Turn:
        hand.getBettingActions()->getTurn().setLastRaiser(player);
        break;
    case GameState::River:
        hand.getBettingActions()->getRiver().setLastRaiser(player);
        break;
    default:
        break;
    }
}

void ActionApplier::finalizeAction(Hand& hand, player::Player& player, const PlayerAction& actionForHistory, pkt::core::Logger& logger)
{
    // Record action in hand-level chronological history
    hand.getBettingActions()->recordPlayerAction(hand.getStateManager()->getGameState(), actionForHistory);

    player.processAction(actionForHistory, hand);

    updateActingPlayersList(hand.getActingPlayersListMutable(), logger);

    if (hand.getEvents().onPlayerActed)
    {
        hand.getEvents().onPlayerActed(actionForHistory);
    }
}

} // namespace pkt::core
