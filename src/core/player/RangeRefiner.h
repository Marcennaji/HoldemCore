#pragma once
#include <string>
#include <vector>

namespace pkt::core::player
{

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
