#include "PreflopState.h"
#include "FlopState.h"
#include "GameEvents.h"
#include "Hand.h"
#include "core/engine/model/PlayerAction.h"

namespace pkt::core
{

PreflopState::PreflopState(GameEvents& events) : myEvents(events)
{
}

void PreflopState::enter(IHand& hand)
{
    hand.postBlindsFsm();
    hand.prepareBettingRoundFsm();

    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(GameStatePreflop);
}

void PreflopState::exit(IHand& hand)
{
    // Nothing needed for now
}
bool PreflopState::canProcessAction(const IHand& hand, const PlayerAction action) const
{
    /*auto player = getPlayerById(action.playerId);

    // Player must be in the hand
    if (player.isFolded())
        return false;

    // It's not this player's turn
    if (hand.currentPlayerId() != action.playerId)
        return false;

    const int callAmount = hand.amountToCall(player.id());

    switch (action.type)
    {
    case ActionType::Fold:
        return true; // Always allowed
    case ActionType::Check:
        return callAmount == 0;
    case ActionType::Call:
        return callAmount > 0 && player.stack() >= callAmount;
    case ActionType::Bet:
        // Only allowed if no previous bet
        return callAmount == 0 && action.amount > 0 && action.amount <= player.stack();
    case ActionType::Raise:
        // Raise must be above minRaise and within stack
        return callAmount > 0 && action.amount >= hand.minRaiseAmount() && action.amount <= player.stack();
    case ActionType::Allin:
        return player.stack() > 0;
    default:
        return false;
    }*/
}
std::unique_ptr<IBettingRoundStateFsm> PreflopState::processAction(IHand& hand, PlayerAction action)
{
    if (!canProcessAction(hand, action))
        return nullptr;

    hand.applyActionFsm(action);

    if (isRoundComplete(hand))
    {
        exit(hand);
        return std::make_unique<FlopState>(myEvents);
    }

    hand.advanceToNextPlayerFsm();
    return nullptr;
}

GameState PreflopState::getGameState() const
{
    return GameStatePreflop;
}

std::string PreflopState::getStateName() const
{
    return "Preflop";
}

bool PreflopState::isRoundComplete(const IHand& hand) const
{
    return hand.isBettingRoundCompleteFsm();
}

void PreflopState::logStateInfo(const IHand& hand) const
{
    // Optional debug logging
}

} // namespace pkt::core