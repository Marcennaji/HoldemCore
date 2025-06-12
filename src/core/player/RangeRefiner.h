#pragma once
#include <string>
#include <vector>

namespace pkt::core::player
{

class RangeRefiner
{
  public:
    // Subtract ranges from the origin ranges, considering the board
    static std::string substractRange(const std::string originRanges, const std::string rangesToSubstract,
                                      const std::string board);

    // Preprocess the board to remove conflicting atomic ranges
    static std::string preprocessBoardConflicts(const std::string& originRanges, const std::string& board);

    // Process a single origin range
    static void processSingleOriginRange(const std::string& singleOriginRange, const std::string& rangesToSubstract,
                                         std::string& newRange);

    // Remove previously included hand from the range
    static void removePreviouslyIncludedHand(const std::string& originHand, std::string& newRange);

    // Process atomic ranges within a single origin range
    static void processAtomicRanges(const std::string& singleOriginRange, const std::string& rangesToSubstract,
                                    std::string& newRange);

    // Process a real hand (specific combination)
    static void processRealHand(const std::string& atomicOriginRange, const std::string& rangesToSubstract,
                                std::string& newRange);

    // Process a range (suited or unsuited)
    static void processRange(const std::string& atomicOriginRange, const std::string& rangesToSubstract,
                             std::string& newRange);

    // Check if a range is suited
    static bool isSuitedRange(const std::vector<std::string>& handsInAtomicRange);

    // Process suited ranges
    static void processSuitedRange(const std::vector<std::string>& handsInAtomicRange,
                                   const std::string& rangesToSubstract, std::string& newRange,
                                   const std::string& atomicOriginRange);

    // Process unsuited ranges
    static void processUnsuitedRange(const std::vector<std::string>& handsInAtomicRange,
                                     const std::string& rangesToSubstract, std::string& newRange,
                                     const std::string& atomicOriginRange);
    static bool isRangeInSubstractList(const std::string& range, const std::string& rangesToSubstract);
    static std::string joinRanges(const std::vector<std::string>& ranges);
};

} // namespace pkt::core::player
