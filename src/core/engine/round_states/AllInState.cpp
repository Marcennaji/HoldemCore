#include "AllInState.h"
#include "PostRiverState.h"
#include "core/engine/Hand.h"

namespace pkt::core
{

void AllInState::enter(Hand& hand)
{
    GlobalServices::instance().logger()->info("Entering AllIn state");

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

void AllInState::exit(Hand& hand)
{
    GlobalServices::instance().logger()->info("Exiting AllIn state");
    // Clean up all-in specific state
}

std::unique_ptr<IBettingRoundState> AllInState::processAction(Hand& hand, PlayerAction action)
{
    // In all-in state, no player actions are typically processed
    // This state automatically transitions to showdown

    if (showdownReady)
    {
        return checkForTransition(hand);
    }

    return nullptr;
}

bool AllInState::isRoundComplete(const Hand& hand) const
{
    return showdownReady;
}

bool AllInState::canProcessAction(const Hand& hand, PlayerAction action) const
{
    // No actions can be processed in all-in state
    return false;
}

void AllInState::logStateInfo(const Hand& hand) const
{
    GlobalServices::instance().logger()->info("AllIn State - Entry state: " + std::to_string(entryState) +
                                              ", Showdown ready: " + std::to_string(showdownReady));
}

// Private method implementations
void AllInState::handleAllInSituation(Hand& hand)
{
    // Extract from Hand::switchRounds() all-in condition handling
    // This will be implemented in Phase 2

    // Mark hand as all-in
    hand.setAllInCondition(true);
    hand.getBoard()->setAllInCondition(true);
}

void AllInState::dealRemainingCards(Hand& hand)
{
    // Extract from Hand::switchRounds() - deal remaining board cards
    // This will be implemented in Phase 2

    // Based on current state, deal cards to complete the board
    // If preflop all-in: deal flop, turn, river
    // If flop all-in: deal turn, river
    // If turn all-in: deal river
    // If river all-in: no additional cards
}

void AllInState::collectPots(Hand& hand)
{
    // Extract from Hand::switchRounds() pot collection logic
    // This will be implemented in Phase 2

    hand.getBoard()->collectPot();

    if (hand.getEvents().onPotUpdated)
    {
        hand.getEvents().onPotUpdated(hand.getBoard()->getPot());
    }

    if (hand.getEvents().onRefreshSet)
    {
        hand.getEvents().onRefreshSet();
    }
}

void AllInState::flipCards(Hand& hand)
{
    // Extract from Hand::switchRounds() card flipping logic
    // This will be implemented in Phase 2

    if (hand.getEvents().onFlipHoleCardsAllIn)
    {
        hand.getEvents().onFlipHoleCardsAllIn();
    }
}

std::unique_ptr<IBettingRoundState> AllInState::checkForTransition(Hand& hand)
{
    // All-in always goes to showdown
    return std::make_unique<PostRiverState>();
}

void AllInState::determineEntryState(Hand& hand)
{
    // Determine which state we entered from
    entryState = hand.getCurrentRound();
    showdownReady = true; // All-in is always ready for showdown
}

} // namespace pkt::core
