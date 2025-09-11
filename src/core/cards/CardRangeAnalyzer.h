// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <string>

namespace pkt::core::cards
{

/**
 * @brief Analyzes if cards fall within specified ranges
 *
 * This module handles card range analysis including exact pairs, suited hands,
 * offsuited hands, and range combinations.
 */
class CardRangeAnalyzer
{
  public:
    /**
     * @brief Check if two cards fall within a specified range string
     *
     * @param card1 First card (e.g., "Ah")
     * @param card2 Second card (e.g., "Kd")
     * @param ranges Comma-separated range string (e.g., "AA,KQs,AKo+")
     * @return true if cards match any range, false otherwise
     */
    static bool isCardsInRange(const std::string& card1, const std::string& card2, const std::string& ranges);

    /**
     * @brief Count board cards higher than the specified card
     *
     * @param stringBoard Space-separated board cards (e.g., "Ah Kd Qc")
     * @param card Reference card to compare against
     * @return Number of board cards with higher value
     */
    static int getBoardCardsHigherThan(const std::string& stringBoard, const std::string& card);

  private:
    // Internal helper functions for range analysis
    static std::string getFakeCard(char c);
    static void ensureHighestCard(std::string& card1, std::string& card2);
    static bool isValidRange(const std::string& token);
    static bool isExactPair(const char* c1, const char* c2, const char* range);
    static bool isExactSuitedHand(const char* c1, const char* c2, const char* range);
    static bool isExactOffsuitedHand(const char* c1, const char* c2, const char* range);
    static bool isPairAndAbove(const std::string& card1, const std::string& card2, const char* range);
    static bool isOffsuitedAndAbove(const std::string& card1, const std::string& card2, const char* c1, const char* c2,
                                    const char* range);
    static bool isSuitedAndAbove(const std::string& card1, const std::string& card2, const char* c1, const char* c2,
                                 const char* range);
    static bool isExactHand(const std::string& card1, const std::string& card2, const char* range);
};

} // namespace pkt::core::cards
