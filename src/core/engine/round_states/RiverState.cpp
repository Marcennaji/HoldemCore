#include "RiverState.h"
#include "GameEvents.h"
#include "Hand.h"
#include "PostRiverState.h"
#include "core/engine/model/PlayerAction.h"

namespace pkt::core
{

RiverState::RiverState(GameEvents& events) : myEvents(events)
{
}

void RiverState::enter(IHand& hand)
{
    hand.dealRiverFsm();
    hand.prepareBettingRoundFsm();

    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(GameState::GameStateRiver);
}

void RiverState::exit(IHand& hand)
{
    // Nothing needed for now
}

std::unique_ptr<IBettingRoundStateFsm> RiverState::processAction(IHand& hand, PlayerAction action)
{
    if (!canProcessAction(hand, action))
        return nullptr;

    hand.applyActionFsm(action);

    if (isRoundComplete(hand))
    {
        exit(hand);
        return std::make_unique<PostRiverState>(myEvents);
    }

    hand.advanceToNextPlayerFsm();
    return nullptr;
}

GameState RiverState::getGameState() const
{
    return GameState::GameStateRiver;
}

std::string RiverState::getStateName() const
{
    return "River";
}

bool RiverState::isRoundComplete(const IHand& hand) const
{
    return hand.isBettingRoundCompleteFsm();
}

bool RiverState::canProcessAction(const IHand& hand, PlayerAction action) const
{
    return hand.canAcceptActionFsm(action);
}

void RiverState::logStateInfo(const IHand& hand) const
{
    // Optional debug logging
}

} // namespace pkt::core