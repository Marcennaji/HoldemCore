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

bool PreflopState::canProcessAction(const IHand& hand, PlayerAction action) const
{
    return hand.canAcceptActionFsm(action);
}

void PreflopState::logStateInfo(const IHand& hand) const
{
    // Optional debug logging
}

} // namespace pkt::core