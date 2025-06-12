#pragma once

#include <core/engine/model/PlayerPosition.h>
#include <core/engine/model/PlayerStatistics.h>

#include <vector>

namespace pkt::core::player
{
struct CurrentHandContext;

class PreflopRangeCalculator
{
  public:
    float getPreflopCallingRange(const CurrentHandContext& context, bool deterministic = false) const;
    float getPreflopRaisingRange(const CurrentHandContext& context, bool deterministic = false) const;

    int getRange(PlayerPosition p, int nbPlayers) const;
    void initializeRanges(const int utgHeadsUpRange, const int utgFullTableRange);

  private:
    // Vector index is player position, value is range %
    std::vector<int> UTG_STARTING_RANGE;
    std::vector<int> UTG_PLUS_ONE_STARTING_RANGE;
    std::vector<int> UTG_PLUS_TWO_STARTING_RANGE;
    std::vector<int> MIDDLE_STARTING_RANGE;
    std::vector<int> MIDDLE_PLUS_ONE_STARTING_RANGE;
    std::vector<int> LATE_STARTING_RANGE;
    std::vector<int> CUTOFF_STARTING_RANGE;
    std::vector<int> BUTTON_STARTING_RANGE;
    std::vector<int> SB_STARTING_RANGE;
    std::vector<int> BB_STARTING_RANGE;

    float adjustForLimpers(float callingRange) const;
    float clampRange(float callingRange) const;
    float adjustForRaises(const CurrentHandContext& context, float callingRange) const;
    float adjustForRaiserStats(float callingRange, const PreflopStatistics& raiserStats, int nbRaises, int nbPlayers,
                               PlayerPosition myPosition, int nbRunningPlayers) const;
    float adjustForNoStats(float callingRange, int nbRaises) const;
    float adjustForBigBet(float callingRange, int potOdd, int myCash, int highestSet, int mySet, int smallBlind) const;
};

} // namespace pkt::core::player