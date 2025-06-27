#include "PreflopState.h"
#include <core/services/GlobalServices.h>
#include "core/engine/model/ButtonState.h"
// #include "AllInState.h"     // Forward declaration needed
// #include "FlopState.h"      // Forward declaration needed
// #include "PostRiverState.h" // Forward declaration needed

namespace pkt::core
{

void PreflopState::enter(Hand& hand)
{
    GlobalServices::instance().logger()->info("Entering Preflop state");

    // Initialize betting round
    initializeBettingRound(hand);

    // Post blinds - extract from existing Hand::setBlinds()
    postBlinds(hand);

    // Set first player to act
    setFirstPlayerToAct(hand);

    // Fire event
    if (hand.getEvents().onStartPreflop)
    {
        hand.getEvents().onStartPreflop();
    }

    logStateInfo(hand);
}

void PreflopState::exit(Hand& hand)
{
    GlobalServices::instance().logger()->info("Exiting Preflop state");

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

std::unique_ptr<IBettingRoundState> PreflopState::processAction(Hand& hand, PlayerAction action)
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
    GlobalServices::instance().logger()->info("Preflop State - Highest bet: " + std::to_string(highestBet) +
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

    auto runningPlayers = hand.getRunningPlayerList();

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
    auto activePlayers = hand.getActivePlayerList();

    // Simple implementation - find player after big blind
    for (auto it = activePlayers->begin(); it != activePlayers->end(); ++it)
    {
        if ((*it)->getButton() == ButtonBigBlind)
        {
            ++it;
            if (it == activePlayers->end())
            {
                it = activePlayers->begin();
            }
            currentPlayerToAct = (*it)->getId();
            break;
        }
    }
}

} // namespace pkt::core
