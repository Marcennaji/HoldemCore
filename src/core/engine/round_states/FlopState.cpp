#include "FlopState.h"
#include "AllInState.h"
#include "PostRiverState.h"
#include "TurnState.h"
#include "core/engine/Hand.h"

namespace pkt::core
{

void FlopState::enter(Hand& hand)
{
    GlobalServices::instance().logger()->verbose("Entering Flop state");

    // Initialize betting round
    initializeBettingRound(hand);

    // Deal flop cards
    dealFlopCards(hand);

    // Set first player to act
    setFirstPlayerToAct(hand);

    // Fire event
    if (hand.getEvents().onStartFlop)
    {
        hand.getEvents().onStartFlop();
    }

    logStateInfo(hand);
}

void FlopState::exit(Hand& hand)
{
    GlobalServices::instance().logger()->verbose("Exiting Flop state");
    // Clean up any flop-specific state
}

std::unique_ptr<IBettingRoundStateFsm> FlopState::processAction(Hand& hand, PlayerAction action)
{
    if (!canProcessAction(hand, action))
    {
        GlobalServices::instance().logger()->error("Invalid action in flop state");
        return nullptr;
    }

    // Process the betting action
    processPlayerBetting(hand, action);

    // Check if this completes the betting round
    if (isBettingComplete(hand))
    {
        bettingComplete = true;
        return checkForTransition(hand);
    }

    // Move to next player
    advanceToNextPlayer(hand);

    return nullptr; // No state transition yet
}

bool FlopState::isRoundComplete(const Hand& hand) const
{
    return bettingComplete;
}

bool FlopState::canProcessAction(const Hand& hand, PlayerAction action) const
{
    return validateAction(hand, action);
}

void FlopState::logStateInfo(const Hand& hand) const
{
    GlobalServices::instance().logger()->verbose("Flop State - Highest bet: " + std::to_string(highestBet) +
                                                 ", Players acted: " + std::to_string(playersActedCount) +
                                                 ", Current player: " + std::to_string(currentPlayerToAct));
}

// Private method implementations
void FlopState::initializeBettingRound(Hand& hand)
{
    highestBet = 0; // Fresh betting round
    bettingComplete = false;
    lastRaiserId = -1;
    playersActedCount = 0;
}

void FlopState::dealFlopCards(Hand& hand)
{
    // Extract from Hand::dealBoardCards() - deal first 3 cards
    // This will be implemented in Phase 2
}

void FlopState::setFirstPlayerToAct(Hand& hand)
{
    // Extract from BettingRound::handleFirstRun() - find first player after dealer
    // This will be implemented in Phase 2
}

void FlopState::processPlayerBetting(Hand& hand, PlayerAction action)
{
    // Extract from BettingRound logic
    // This will be implemented in Phase 2
}

void FlopState::advanceToNextPlayer(Hand& hand)
{
    // Extract from BettingRound::run() - next player logic
    // This will be implemented in Phase 2
}

std::unique_ptr<IBettingRoundStateFsm> FlopState::checkForTransition(Hand& hand)
{
    // Check for all-in condition
    if (checkAllInCondition(hand))
    {
        return std::make_unique<AllInState>();
    }

    // Check if only one player remains
    if (checkSinglePlayerRemaining(hand))
    {
        return std::make_unique<PostRiverState>();
    }

    // Normal transition to turn
    return std::make_unique<TurnState>();
}

bool FlopState::checkSinglePlayerRemaining(const Hand& hand) const
{
    // Extract from Hand::resolveHandConditions() logic
    // This will be implemented in Phase 2
    return false;
}

bool FlopState::checkAllInCondition(const Hand& hand) const
{
    // Extract from Hand::resolveHandConditions() all-in logic
    // This will be implemented in Phase 2
    return false;
}

bool FlopState::validateAction(const Hand& hand, PlayerAction action) const
{
    // Extract from existing betting validation logic
    // This will be implemented in Phase 2
    return true;
}

bool FlopState::isBettingComplete(const Hand& hand) const
{
    // Extract from BettingRound::allBetsAreDone()
    // This will be implemented in Phase 2
    return false;
}

void FlopState::updatePlayerLists(Hand& hand)
{
    // Extract from Hand::resolveHandConditions() - player list refresh
    // This will be implemented in Phase 2
}

int FlopState::getNextPlayerToAct(const Hand& hand) const
{
    // Extract from BettingRound::run() - next player logic
    // This will be implemented in Phase 2
    return -1;
}

} // namespace pkt::core
