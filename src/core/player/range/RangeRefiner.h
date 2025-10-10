// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once
#include <string>
#include <vector>

namespace pkt::core::player
{

/**
 * @brief Helper class for subtracting implausible hole cards from player ranges.
 * 
 * Refines opponent hand ranges by removing implausible combinations based on
 * the player's playing style and observed actions during the current hand,
 * producing more accurate range estimates for strategic decision making.
 */
class RangeRefiner
{
  public:
    // deduce the possible ranges from the origin ranges, considering the board (if any) and the ranges to substract
    static std::string deduceRange(const std::string originRanges, const std::string rangesToSubstract,
                                   const std::string board = "");

  private:
    // Process real hands
    static void processRealHands(const std::vector<std::string>& hands, const std::string& rangesToSubstract,
                                 const std::string& board, std::string& newRange);

    // Process suited ranges
    static void processSuitedRanges(const std::vector<std::string>& hands, const std::string& rangesToSubstract,
                                    const std::string& board, const std::string& atomicOriginRange,
                                    std::string& newRange);

    // Process unsuited ranges
    static void processUnsuitedRanges(const std::vector<std::string>& hands, const std::string& rangesToSubstract,
                                      const std::string& board, const std::string& atomicOriginRange,
                                      std::string& newRange);
};

} // namespace pkt::core::player
