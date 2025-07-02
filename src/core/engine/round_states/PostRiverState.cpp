#include "PostRiverState.h"
#include "core/engine/Hand.h"
#include "core/player/HumanPlayer.h"

namespace pkt::core
{

void PostRiverState::enter(Hand& hand)
{
    GlobalServices::instance().logger()->verbose("Entering PostRiver state");

    // Fire event
    if (hand.getEvents().onStartPostRiver)
    {
        hand.getEvents().onStartPostRiver();
    }

    // Run showdown
    runShowdown(hand);

    logStateInfo(hand);
}

void PostRiverState::exit(Hand& hand)
{
    GlobalServices::instance().logger()->verbose("Exiting PostRiver state - Hand complete");
    // Clean up post-river state
}

std::unique_ptr<IBettingRoundStateFsm> PostRiverState::processAction(Hand& hand, PlayerAction action)
{
    // PostRiver doesn't process betting actions
    // This is the final state - hand is complete
    return nullptr;
}

bool PostRiverState::isRoundComplete(const Hand& hand) const
{
    return showdownComplete;
}

bool PostRiverState::canProcessAction(const Hand& hand, PlayerAction action) const
{
    // No betting actions in post-river
    return false;
}

void PostRiverState::logStateInfo(const Hand& hand) const
{
    GlobalServices::instance().logger()->verbose(
        "PostRiver State - Highest cards value: " + std::to_string(highestCardsValue) +
        ", Showdown complete: " + std::to_string(showdownComplete));
}

// Private method implementations
void PostRiverState::runShowdown(Hand& hand)
{
    // Extract from BettingRoundPostRiver::postRiverRun()
    // This will be implemented in Phase 2

    determineWinners(hand);
    showCards(hand);
    distributePots(hand);
    updateStatistics(hand);
    handleHandPause(hand);

    showdownComplete = true;
}

void PostRiverState::determineWinners(Hand& hand)
{
    // Extract from BettingRoundPostRiver::postRiverRun() - winner determination
    // This will be implemented in Phase 2

    calculateHighestCardsValue(hand);
    hand.getBoard()->determinePlayerNeedToShowCards();
}

void PostRiverState::showCards(Hand& hand)
{
    // Extract from BettingRoundPostRiver::postRiverRun() - card showing logic
    // This will be implemented in Phase 2

    if (hand.getCardsShown())
    {
        // Show cards for players who didn't fold preflop
        auto seatsList = hand.getSeatsList();
        for (auto it = seatsList->begin(); it != seatsList->end(); ++it)
        {
            // Check if player has preflop actions and didn't fold
            if ((*it)->getCurrentHandActions().getPreflopActions().size() > 0 &&
                (*it)->getCurrentHandActions().getPreflopActions().at(0) != PlayerActionFold)
            {
                if (hand.getEvents().onShowHoleCards)
                {
                    hand.getEvents().onShowHoleCards((*it)->getId());
                }
            }
        }
    }
}

void PostRiverState::distributePots(Hand& hand)
{
    // Extract from BettingRoundPostRiver::postRiverRun() - pot distribution
    // This will be implemented in Phase 2

    hand.getBoard()->distributePot();
    hand.getBoard()->setPot(0);
}

void PostRiverState::updateStatistics(Hand& hand)
{
    // Extract from BettingRoundPostRiver::postRiverRun() - statistics update
    // This will be implemented in Phase 2

    GlobalServices::instance().rankingStore()->updateRankingGameLosers(hand.getSeatsList());
    GlobalServices::instance().rankingStore()->updateRankingGameWinner(hand.getSeatsList());
    GlobalServices::instance().playersStatisticsStore()->updatePlayersStatistics(hand.getSeatsList());
}

void PostRiverState::handleHandPause(Hand& hand)
{
    // Extract from BettingRoundPostRiver::postRiverRun() - hand pause logic
    // This will be implemented in Phase 2

    if (shouldPauseHand(hand) && hand.getEvents().onPauseHand)
    {
        hand.getEvents().onPauseHand();
    }

    if (hand.getEvents().onPostRiverRunAnimation)
    {
        hand.getEvents().onPostRiverRunAnimation();
    }
}

void PostRiverState::calculateHighestCardsValue(Hand& hand)
{
    // Extract from BettingRoundPostRiver::postRiverRun() - highest cards calculation
    // This will be implemented in Phase 2

    auto seatsList = hand.getSeatsList();
    for (auto it = seatsList->begin(); it != seatsList->end(); ++it)
    {
        if ((*it)->getAction() != PlayerActionFold && (*it)->getCardsValueInt() > highestCardsValue)
        {
            highestCardsValue = (*it)->getCardsValueInt();
        }
    }
}

bool PostRiverState::shouldPauseHand(Hand& hand) const
{
    // Extract from BettingRoundPostRiver::postRiverRun() - pause logic
    // This will be implemented in Phase 2

    bool humanPlayerInvolved = false;
    int nonFoldPlayerCount = countNonFoldedPlayers(hand);

    auto seatsList = hand.getSeatsList();
    for (auto it = seatsList->begin(); it != seatsList->end(); ++it)
    {
        if ((*it)->getAction() != PlayerActionFold && (*it)->getName() == pkt::core::player::HumanPlayer::getName())
        {
            humanPlayerInvolved = true;
            break;
        }
    }

    return humanPlayerInvolved && nonFoldPlayerCount > 1;
}

int PostRiverState::countNonFoldedPlayers(Hand& hand) const
{
    // Extract from BettingRoundPostRiver::postRiverRun() - player counting
    // This will be implemented in Phase 2

    int count = 0;
    auto seatsList = hand.getSeatsList();
    for (auto it = seatsList->begin(); it != seatsList->end(); ++it)
    {
        if ((*it)->getAction() != PlayerActionFold)
        {
            count++;
        }
    }
    return count;
}

} // namespace pkt::core
