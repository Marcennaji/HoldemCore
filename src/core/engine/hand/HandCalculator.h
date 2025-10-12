// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/ports/Logger.h"
#include <string>

namespace pkt::core
{

// Forward declarations  
class Hand;

/**
 * @brief Handles mathematical calculations and computations for poker hands.
 * 
 * This class extracts calculation responsibilities from the Hand class,
 * providing focused mathematical operations without coupling to game state.
 * 
 * Responsibilities:
 * - M-ratio calculations (tournament chip management)
 * - Pot odds calculations  
 * - Mathematical utility functions
 */
class HandCalculator
{
  public:
    HandCalculator(Logger& logger);
    ~HandCalculator() = default;

    // M-ratio calculation (tournament chip management)
    float calculateM(int cash, int smallBlind) const;

    // Pot odds calculation
    int calculatePotOdd(int playerCash, int playerSet, int pot, int sets, int roundHighestSet, Logger& logger) const;

  private:
    Logger* m_logger;
};

} // namespace pkt::core