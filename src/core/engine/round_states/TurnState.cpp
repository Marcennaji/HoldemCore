#include "TurnState.h"
#include "AllInState.h"
#include "PostRiverState.h"
#include "RiverState.h"
#include "core/engine/Hand.h"

namespace pkt::core
{

void TurnState::enter(Hand& hand)
{
    GlobalServices::instance().logger()->verbose("Entering Turn state");

    // Initialize betting round
    initializeBettingRound(hand);

    // Deal turn card
    dealTurnCard(hand);

    // Set first player to act
    setFirstPlayerToAct(hand);

    // Fire event
    if (hand.getEvents().onStartTurn)
    {
        hand.getEvents().onStartTurn();
    }

    logStateInfo(hand);
}

void TurnState::exit(Hand& hand)
{
    GlobalServices::instance().logger()->verbose("Exiting Turn state");
    // Clean up any turn-specific state
}

std::unique_ptr<IBettingRoundStateFsm> TurnState::processAction(Hand& hand, PlayerAction action)
{
    if (!canProcessAction(hand, action))
    {
        GlobalServices::instance().logger()->error("Invalid action in turn state");
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

bool TurnState::isRoundComplete(const Hand& hand) const
{
    return bettingComplete;
}

bool TurnState::canProcessAction(const Hand& hand, PlayerAction action) const
{
    return validateAction(hand, action);
}

void TurnState::logStateInfo(const Hand& hand) const
{
    GlobalServices::instance().logger()->verbose("Turn State - Highest bet: " + std::to_string(highestBet) +
                                                 ", Players acted: " + std::to_string(playersActedCount) +
                                                 ", Current player: " + std::to_string(currentPlayerToAct));
}

// Private method implementations
void TurnState::initializeBettingRound(Hand& hand)
{
    highestBet = 0; // Fresh betting round
    bettingComplete = false;
    lastRaiserId = -1;
    playersActedCount = 0;
}

void TurnState::dealTurnCard(Hand& hand)
{
    // Extract from Hand::dealBoardCards() - deal 4th card
    // This will be implemented in Phase 2
}

void TurnState::setFirstPlayerToAct(Hand& hand)
{
    // Extract from BettingRound::handleFirstRun() - find first player after dealer
    // This will be implemented in Phase 2
}

void TurnState::processPlayerBetting(Hand& hand, PlayerAction action)
{
    // Extract from BettingRound logic
    // This will be implemented in Phase 2
}

void TurnState::advanceToNextPlayer(Hand& hand)
{
    // Extract from BettingRound::run() - next player logic
    // This will be implemented in Phase 2
}

std::unique_ptr<IBettingRoundStateFsm> TurnState::checkForTransition(Hand& hand)
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

    // Normal transition to river
    return std::make_unique<RiverState>();
}

bool TurnState::checkSinglePlayerRemaining(const Hand& hand) const
{
    // Extract from Hand::resolveHandConditions() logic
    // This will be implemented in Phase 2
    return false;
}

bool TurnState::checkAllInCondition(const Hand& hand) const
{
    // Extract from Hand::resolveHandConditions() all-in logic
    // This will be implemented in Phase 2
    return false;
}

bool TurnState::validateAction(const Hand& hand, PlayerAction action) const
{
    // Extract from existing betting validation logic
    // This will be implemented in Phase 2
    return true;
}

bool TurnState::isBettingComplete(const Hand& hand) const
{
    // Extract from BettingRound::allBetsAreDone()
    // This will be implemented in Phase 2
    return false;
}

void TurnState::updatePlayerLists(Hand& hand)
{
    // Extract from Hand::resolveHandConditions() - player list refresh
    // This will be implemented in Phase 2
}

int TurnState::getNextPlayerToAct(const Hand& hand) const
{
    // Extract from BettingRound::run() - next player logic
    // This will be implemented in Phase 2
    return -1;
}

} // namespace pkt::core
