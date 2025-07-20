#include "AllInState.h"
#include "PostRiverState.h"
#include "core/engine/Hand.h"

namespace pkt::core
{
AllInState::AllInState(GameEvents& events) : myEvents(events)
{
}

void AllInState::enter(IHand& hand)
{
    GlobalServices::instance().logger()->verbose("Entering AllIn state");

    // Determine which state we entered from
    determineEntryState(hand);

    // Handle all-in situation
    handleAllInSituation(hand);

    // Collect pots
    collectPots(hand);

    // Flip cards for all-in animation
    flipCards(hand);

    // Deal remaining cards
    dealRemainingCards(hand);

    logStateInfo(hand);
}

void AllInState::exit(IHand& hand)
{
    GlobalServices::instance().logger()->verbose("Exiting AllIn state");
    // Clean up all-in specific state
}

std::unique_ptr<IBettingRoundStateFsm> AllInState::processAction(IHand& hand, PlayerAction action)
{
    // In all-in state, no player actions are typically processed
    // This state automatically transitions to showdown

    if (showdownReady)
    {
        return checkForTransition(hand);
    }

    return nullptr;
}

bool AllInState::isRoundComplete(const IHand& hand) const
{
    return showdownReady;
}

bool AllInState::canProcessAction(const IHand& hand, PlayerAction action) const
{
    // No actions can be processed in all-in state
    return false;
}

void AllInState::logStateInfo(const IHand& hand) const
{
    GlobalServices::instance().logger()->verbose("AllIn State - Entry state: " + std::to_string(entryState) +
                                                 ", Showdown ready: " + std::to_string(showdownReady));
}

// Private method implementations
void AllInState::handleAllInSituation(IHand& hand)
{
    // Extract from Hand::resolveHandConditions() all-in condition handling
    // This will be implemented in Phase 2

    // Mark hand as all-in
    hand.setAllInCondition(true);
    hand.getBoard()->setAllInCondition(true);
}

void AllInState::dealRemainingCards(IHand& hand)
{
    // Extract from Hand::resolveHandConditions() - deal remaining board cards
    // This will be implemented in Phase 2

    // Based on current state, deal cards to complete the board
    // If preflop all-in: deal flop, turn, river
    // If flop all-in: deal turn, river
    // If turn all-in: deal river
    // If river all-in: no additional cards
}

void AllInState::collectPots(IHand& hand)
{
    // Extract from Hand::resolveHandConditions() pot collection logic
    // This will be implemented in Phase 2

    hand.getBoard()->collectPot();

    if (myEvents.onPotUpdated)
    {
        myEvents.onPotUpdated(hand.getBoard()->getPot());
    }
}

void AllInState::flipCards(IHand& hand)
{
    // Extract from Hand::resolveHandConditions() card flipping logic
    // This will be implemented in Phase 2

    if (myEvents.onFlipHoleCardsAllIn)
    {
        myEvents.onFlipHoleCardsAllIn();
    }
}

std::unique_ptr<IBettingRoundStateFsm> AllInState::checkForTransition(IHand& hand)
{
    // All-in always goes to showdown
    return std::make_unique<PostRiverState>(myEvents);
}

void AllInState::determineEntryState(IHand& hand)
{
    // Determine which state we entered from
    entryState = hand.getCurrentRoundState();
    showdownReady = true; // All-in is always ready for showdown
}

} // namespace pkt::core
