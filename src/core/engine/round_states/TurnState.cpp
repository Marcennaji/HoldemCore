#include "TurnState.h"
#include "GameEvents.h"
#include "Hand.h"
#include "RiverState.h"
#include "core/engine/model/PlayerAction.h"

namespace pkt::core
{

TurnState::TurnState(GameEvents& events) : myEvents(events)
{
}

void TurnState::enter(IHand& hand)
{
    hand.dealTurnFsm();
    hand.prepareBettingRoundFsm();

    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(GameState::GameStateTurn);
}

void TurnState::exit(IHand& hand)
{
    // Nothing needed for now
}

std::unique_ptr<IBettingRoundStateFsm> TurnState::processAction(IHand& hand, PlayerAction action)
{
    if (!canProcessAction(hand, action))
        return nullptr;

    hand.applyActionFsm(action);

    if (isRoundComplete(hand))
    {
        exit(hand);
        return std::make_unique<RiverState>(myEvents);
    }

    hand.advanceToNextPlayerFsm();
    return nullptr;
}

GameState TurnState::getGameState() const
{
    return GameState::GameStateTurn;
}

std::string TurnState::getStateName() const
{
    return "Turn";
}

bool TurnState::isRoundComplete(const IHand& hand) const
{
    return hand.isBettingRoundCompleteFsm();
}

bool TurnState::canProcessAction(const IHand& hand, PlayerAction action) const
{
    return hand.canAcceptActionFsm(action);
}

void TurnState::logStateInfo(const IHand& hand) const
{
    // Optional debug logging
}

} // namespace pkt::core