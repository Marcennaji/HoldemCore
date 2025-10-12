// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/ports/HandEvaluationEngine.h"

namespace pkt::core::probability
{

/**
 * @brief Calculates drawing probabilities for poker hands
 *
 * This module handles probability calculations for drawing hands,
 * including pot odds and implicit odds calculations.
 */
class DrawProbabilityCalculator
{
  public:
    /**
     * @brief Calculate the probability of improving to a winning hand
     *
     * @param postFlopAnalysis Analysis flags containing outs information
     * @return Percentage chance of winning (0-100)
     */
    static int getDrawingProbability(const pkt::core::PostFlopAnalysisFlags& postFlopAnalysis);

    /**
     * @brief Check if drawing probability justifies the pot odds
     *
     * @param postFlopState Analysis flags for the current situation
     * @param potOdd Current pot odds percentage
     * @return true if drawing is profitable, false otherwise
     */
    static bool isDrawingProbOk(const pkt::core::PostFlopAnalysisFlags& postFlopState, int potOdd);

    /**
     * @brief Calculate implicit odds based on game state
     *
     * @param state Current post-flop analysis state
     * @return Implicit odds percentage
     */
    static int getImplicitOdd(const pkt::core::PostFlopAnalysisFlags& state);

  private:
    // Lookup tables for probability calculations
    static const int outsOddsOneCard[];
    static const int outsOddsTwoCard[];
};

} // namespace pkt::core::probability
