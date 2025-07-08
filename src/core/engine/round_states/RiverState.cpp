#include "RiverState.h"
#include "AllInState.h"
#include "PostRiverState.h"
#include "core/engine/Hand.h"

namespace pkt::core
{

void RiverState::enter(Hand& hand)
{
    GlobalServices::instance().logger()->verbose("Entering River state");

    // Initialize betting round
    initializeBettingRound(hand);

    // Deal river card
    dealRiverCard(hand);

    // Set first player to act
    setFirstPlayerToAct(hand);

    // Fire event
    if (hand.getEvents().onBettingRoundStarted)
    {
        hand.getEvents().onBettingRoundStarted(GameStateRiver);
    }

    logStateInfo(hand);
}

void RiverState::exit(Hand& hand)
{
    GlobalServices::instance().logger()->verbose("Exiting River state");
    // Clean up any river-specific state
}

std::unique_ptr<IBettingRoundStateFsm> RiverState::processAction(Hand& hand, PlayerAction action)
{
    if (!canProcessAction(hand, action))
    {
        GlobalServices::instance().logger()->error("Invalid action in river state");
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

bool RiverState::isRoundComplete(const Hand& hand) const
{
    return bettingComplete;
}

bool RiverState::canProcessAction(const Hand& hand, PlayerAction action) const
{
    return validateAction(hand, action);
}

void RiverState::logStateInfo(const Hand& hand) const
{
    GlobalServices::instance().logger()->verbose("River State - Highest bet: " + std::to_string(highestBet) +
                                                 ", Players acted: " + std::to_string(playersActedCount) +
                                                 ", Current player: " + std::to_string(currentPlayerToAct));
}

// Private method implementations
void RiverState::initializeBettingRound(Hand& hand)
{
    highestBet = 0; // Fresh betting round
    bettingComplete = false;
    lastRaiserId = -1;
    playersActedCount = 0;
}

void RiverState::dealRiverCard(Hand& hand)
{
    // Extract from Hand::dealBoardCards() - deal 5th card
    // This will be implemented in Phase 2
}

void RiverState::setFirstPlayerToAct(Hand& hand)
{
    // Extract from BettingRound::handleFirstRun() - find first player after dealer
    // This will be implemented in Phase 2
}

void RiverState::processPlayerBetting(Hand& hand, PlayerAction action)
{
    // Extract from BettingRound logic
    // This will be implemented in Phase 2
}

void RiverState::advanceToNextPlayer(Hand& hand)
{
    // Extract from BettingRound::run() - next player logic
    // This will be implemented in Phase 2
}

std::unique_ptr<IBettingRoundStateFsm> RiverState::checkForTransition(Hand& hand)
{
    // Check for all-in condition
    if (checkAllInCondition(hand))
    {
        return std::make_unique<AllInState>();
    }

    // River always goes to PostRiver (showdown)
    return std::make_unique<PostRiverState>();
}

bool RiverState::checkSinglePlayerRemaining(const Hand& hand) const
{
    // Extract from Hand::resolveHandConditions() logic
    // This will be implemented in Phase 2
    return false;
}

bool RiverState::checkAllInCondition(const Hand& hand) const
{
    // Extract from Hand::resolveHandConditions() all-in logic
    // This will be implemented in Phase 2
    return false;
}

bool RiverState::validateAction(const Hand& hand, PlayerAction action) const
{
    // Extract from existing betting validation logic
    // This will be implemented in Phase 2
    return true;
}

bool RiverState::isBettingComplete(const Hand& hand) const
{
    // Extract from BettingRound::allBetsAreDone()
    // This will be implemented in Phase 2
    return false;
}

void RiverState::updatePlayerLists(Hand& hand)
{
    // Extract from Hand::resolveHandConditions() - player list refresh
    // This will be implemented in Phase 2
}

int RiverState::getNextPlayerToAct(const Hand& hand) const
{
    // Extract from BettingRound::run() - next player logic
    // This will be implemented in Phase 2
    return -1;
}

} // namespace pkt::core
