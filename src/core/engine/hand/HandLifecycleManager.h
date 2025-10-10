// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/Logger.h"
#include "core/interfaces/persistence/PlayersStatisticsStore.h"

namespace pkt::core
{

// Forward declarations
class Hand;
class HandCardDealer;
class HandStateManager;
class HandPlayersManager;

/**
 * @brief Manages hand lifecycle operations for poker hands.
 * 
 * This class extracts lifecycle management responsibilities from the Hand class,
 * providing focused lifecycle coordination without coupling to game logic details.
 * 
 * Responsibilities:
 * - Hand initialization (deck setup, player preparation, state initialization)
 * - Game loop execution (card dealing, state machine coordination)
 * - Hand completion (statistics saving, cleanup operations)
 * - Coordination between lifecycle phases and component managers
 */
class HandLifecycleManager
{
  public:
    HandLifecycleManager(Logger& logger, PlayersStatisticsStore& statisticsStore);
    ~HandLifecycleManager() = default;

    // Main lifecycle interface
    void initialize(Hand& hand);
    void runGameLoop(Hand& hand);
    void end(Hand& hand);
    
  private:
    Logger* m_logger;
    PlayersStatisticsStore* m_statisticsStore;
};

} // namespace pkt::core