#include "PreflopState.h"
#include <core/services/GlobalServices.h>
#include "AllInState.h"
#include "FlopState.h"
#include "PostRiverState.h"
#include "core/engine/Hand.h"
#include "core/engine/model/ButtonState.h"

namespace pkt::core
{

void PreflopState::enter(Hand& hand)
{
    GlobalServices::instance().logger()->verbose("Entering Preflop state");

    // Initialize betting round
    initializeBettingRound(hand);

    // Post blinds - extract from existing Hand::setBlinds()
    postBlinds(hand);

    // Set first player to act
    setFirstPlayerToAct(hand);

    // Fire event
    if (hand.getEvents().onBettingRoundStarted)
    {
        hand.getEvents().onBettingRoundStarted(GameStatePreflop);
    }

    logStateInfo(hand);
}

void PreflopState::exit(Hand& hand)
{
    GlobalServices::instance().logger()->verbose("Exiting Preflop state");

    // Update player lists for next round
    updatePlayerLists(hand);

    // Store preflop raiser info
    hand.setPreflopLastRaiserId(lastRaiserId);

    // Collect pot
    hand.getBoard()->collectSets();

    if (hand.getEvents().onPotUpdated)
    {
        hand.getEvents().onPotUpdated(hand.getBoard()->getPot());
    }
}

std::unique_ptr<IBettingRoundStateFsm> PreflopState::processAction(Hand& hand, PlayerAction action)
{
    if (!canProcessAction(hand, action))
    {
        GlobalServices::instance().logger()->error("Invalid action in preflop state");
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

bool PreflopState::isRoundComplete(const Hand& hand) const
{
    return bettingComplete;
}

bool PreflopState::canProcessAction(const Hand& hand, PlayerAction action) const
{
    return validateAction(hand, action);
}

void PreflopState::logStateInfo(const Hand& hand) const
{
    GlobalServices::instance().logger()->verbose("Preflop State - Highest bet: " + std::to_string(highestBet) +
                                                 ", Players acted: " + std::to_string(playersActedCount) +
                                                 ", Current player: " + std::to_string(currentPlayerToAct));
}

// Private method implementations
void PreflopState::initializeBettingRound(Hand& hand)
{
    highestBet = hand.getSmallBlind() * 2; // Big blind amount
    bettingComplete = false;
    lastRaiserId = -1;
    playersActedCount = 0;
}

void PreflopState::postBlinds(Hand& hand)
{
    // Extract the clean blind posting logic from Hand::setBlinds()
    // This will be much cleaner than the current implementation

    auto runningPlayers = hand.getRunningPlayersList();

    for (auto it = runningPlayers->begin(); it != runningPlayers->end(); ++it)
    {
        // Small blind
        if ((*it)->getButton() == ButtonSmallBlind)
        {
            int blindAmount = hand.getSmallBlind();
            if ((*it)->getCash() <= blindAmount)
            {
                (*it)->setSet((*it)->getCash());
                (*it)->setAction(PlayerActionAllin, 1);
            }
            else
            {
                (*it)->setSet(blindAmount);
            }
        }

        // Big blind
        if ((*it)->getButton() == ButtonBigBlind)
        {
            int blindAmount = 2 * hand.getSmallBlind();
            if ((*it)->getCash() <= blindAmount)
            {
                (*it)->setSet((*it)->getCash());
                (*it)->setAction(PlayerActionAllin, 1);
            }
            else
            {
                (*it)->setSet(blindAmount);
            }
        }
    }
}

void PreflopState::setFirstPlayerToAct(Hand& hand)
{
    // Find first player after big blind
    // This is much cleaner than the current complex logic
    auto seats = hand.getSeatsList();

    // Simple implementation - find player after big blind
    for (auto it = seats->begin(); it != seats->end(); ++it)
    {
        if ((*it)->getButton() == ButtonBigBlind)
        {
            ++it;
            if (it == seats->end())
            {
                it = seats->begin();
            }
            currentPlayerToAct = (*it)->getId();
            break;
        }
    }
}

void PreflopState::processPlayerBetting(Hand& hand, PlayerAction action)
{
    // Extract from BettingRound logic
    // This will be implemented in Phase 2
}

void PreflopState::advanceToNextPlayer(Hand& hand)
{
    // Extract from BettingRound::run() - next player logic
    // This will be implemented in Phase 2
}

std::unique_ptr<IBettingRoundStateFsm> PreflopState::checkForTransition(Hand& hand)
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

    // Normal transition to flop
    return std::make_unique<FlopState>();
}

bool PreflopState::checkSinglePlayerRemaining(const Hand& hand) const
{
    // Extract from Hand::resolveHandConditions() logic
    // This will be implemented in Phase 2
    return false;
}

bool PreflopState::checkAllInCondition(const Hand& hand) const
{
    // Extract from Hand::resolveHandConditions() all-in logic
    // This will be implemented in Phase 2
    return false;
}

bool PreflopState::validateAction(const Hand& hand, PlayerAction action) const
{
    // Extract from existing betting validation logic
    // This will be implemented in Phase 2
    return true;
}

bool PreflopState::isBettingComplete(const Hand& hand) const
{
    // Extract from BettingRound::allBetsAreDone()
    // This will be implemented in Phase 2
    return false;
}

void PreflopState::updatePlayerLists(Hand& hand)
{
    // Extract from Hand::resolveHandConditions() - player list refresh
    // This will be implemented in Phase 2
}

int PreflopState::getNextPlayerToAct(const Hand& hand) const
{
    // Extract from BettingRound::run() - next player logic
    // This will be implemented in Phase 2
    return -1;
}

} // namespace pkt::core
