#include "FlopState.h"
#include "GameEvents.h"
#include "Hand.h"
#include "TurnState.h"
#include "core/engine/model/PlayerAction.h"

namespace pkt::core
{

FlopState::FlopState(GameEvents& events) : myEvents(events)
{
}

void FlopState::enter(IHand& hand)
{
    hand.dealFlopFsm();
    hand.prepareBettingRoundFsm();

    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(GameStateFlop);
}

void FlopState::exit(IHand& hand)
{
    // Nothing needed for now
}

std::unique_ptr<IHandState> FlopState::processAction(IHand& hand, PlayerAction action)
{
    if (!canProcessAction(hand, action))
        return nullptr;

    hand.applyActionFsm(action);

    if (isRoundComplete(hand))
    {
        exit(hand);
        // return std::make_unique<TurnState>(myEvents);
        return nullptr;
    }

    hand.advanceToNextPlayerFsm();
    return nullptr;
}

bool FlopState::isRoundComplete(const IHand& hand) const
{
    return hand.isBettingRoundCompleteFsm();
}

bool FlopState::canProcessAction(const IHand& hand, PlayerAction action) const
{
    return hand.canAcceptActionFsm(action);
}

void FlopState::logStateInfo(const IHand& hand) const
{
    // Optional debug logging
}

} // namespace pkt::core