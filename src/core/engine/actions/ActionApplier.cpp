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
    {
        int amountToCall = currentHighest - playerBet;
        if (player->getCash() < amountToCall)
        {
            amountToCall = player->getCash();
        }

        // If this call uses all remaining cash, treat it as an all-in
        if (amountToCall == player->getCash())
        {
            actionForHistory.type = ActionType::Allin;
            player->setCash(0);
        }

        actionForHistory.amount = amountToCall; // store the actual call amount
        player->addBetAmount(amountToCall);
        hand.fireOnPotUpdated();
        break;
    }

    case ActionType::Raise:
    {
        int raiseIncrement = action.amount - playerBet;
        actionForHistory.amount = raiseIncrement; // store only the increment in history
        player->addBetAmount(raiseIncrement);
        hand.fireOnPotUpdated();
        hand.getBettingActions()->updateRoundHighestSet(action.amount);

        // Record last raiser for the current betting round
        switch (hand.getStateManager()->getGameState())
        {
        case GameState::Preflop:
            hand.getBettingActions()->getPreflop().setLastRaiserId(player->getId());
            break;
        case GameState::Flop:
            hand.getBettingActions()->getFlop().setLastRaiserId(player->getId());
            break;
        case GameState::Turn:
            hand.getBettingActions()->getTurn().setLastRaiserId(player->getId());
            break;
        case GameState::River:
            hand.getBettingActions()->getRiver().setLastRaiserId(player->getId());
            break;
        default:
            break;
        }
        break;
    }

    case ActionType::Bet:
    {
        // For bet, the amount is already the increment
        player->addBetAmount(action.amount);
        hand.fireOnPotUpdated();
        hand.getBettingActions()->updateRoundHighestSet(action.amount);

        // Record last raiser (bettor) for the current betting round
        switch (hand.getStateManager()->getGameState())
        {
        case GameState::Preflop:
            hand.getBettingActions()->getPreflop().setLastRaiserId(player->getId());
            break;
        case GameState::Flop:
            hand.getBettingActions()->getFlop().setLastRaiserId(player->getId());
            break;
        case GameState::Turn:
            hand.getBettingActions()->getTurn().setLastRaiserId(player->getId());
            break;
        case GameState::River:
            hand.getBettingActions()->getRiver().setLastRaiserId(player->getId());
            break;
        default:
            break;
        }
        break;
    }

    case ActionType::Check:
    {
        // No amount change needed
        break;
    }

    case ActionType::Allin:
    {
        int allinIncrement = player->getCash();   // The increment is all remaining cash
        actionForHistory.amount = allinIncrement; // store only the increment in history

        player->addBetAmount(allinIncrement);
        hand.fireOnPotUpdated();
        player->setCash(0);

        if (allinIncrement > currentHighest)
        {
            hand.getBettingActions()->updateRoundHighestSet(allinIncrement);

            // Record last raiser (all-in as raise) for the current betting round
            switch (hand.getStateManager()->getGameState())
            {
            case GameState::Preflop:
                hand.getBettingActions()->getPreflop().setLastRaiserId(player->getId());
                break;
            case GameState::Flop:
                hand.getBettingActions()->getFlop().setLastRaiserId(player->getId());
                break;
            case GameState::Turn:
                hand.getBettingActions()->getTurn().setLastRaiserId(player->getId());
                break;
            case GameState::River:
                hand.getBettingActions()->getRiver().setLastRaiserId(player->getId());
                break;
            default:
                break;
            }
        }
        break;
    }

    case ActionType::None:
    default:
        break;
    }

    player->setAction(hand.getStateManager()->getCurrentState(), actionForHistory);

    // Record action in hand-level chronological history
    hand.getBettingActions()->recordPlayerAction(hand.getStateManager()->getGameState(), actionForHistory);

    updateActingPlayersList(hand.getActingPlayersListMutable());

    if (hand.getEvents().onPlayerActed)
    {
        hand.getEvents().onPlayerActed(action);
    }
}

} // namespace pkt::core
