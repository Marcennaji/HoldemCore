// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "HandLifecycleManager.h"
#include "Hand.h"

namespace pkt::core
{

HandLifecycleManager::HandLifecycleManager(Logger& logger, PlayersStatisticsStore& statisticsStore)
    : m_logger(&logger), m_statisticsStore(&statisticsStore)
{
}

void HandLifecycleManager::initialize(Hand& hand)
{
    m_logger->info("\n----------------------  New hand ----------------------------\n");

    hand.initAndShuffleDeck();
    hand.filterPlayersWithInsufficientCash();
    hand.getPlayersManager()->preparePlayersForNewHand(hand);
    hand.getBettingActions()->getPreflop().setLastRaiser(nullptr);
    hand.getStateManager()->initializeState(hand);
}

void HandLifecycleManager::runGameLoop(Hand& hand)
{
    hand.dealHoleCards(0); // Pass 0 as index, since no board cards dealt yet
    hand.getStateManager()->runGameLoop(hand);
}

void HandLifecycleManager::end(Hand& hand)
{
    m_statisticsStore->savePlayersStatistics(hand.getSeatsList());
}

} // namespace pkt::core