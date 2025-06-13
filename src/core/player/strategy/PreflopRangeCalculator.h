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
    float calculatePreflopCallingRange(CurrentHandContext& context, bool deterministic = false) const;
    float calculatePreflopRaisingRange(CurrentHandContext& context, bool deterministic = false) const;

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

    // calling range helper methods
    float adjustCallForLimpers(float callingRange) const;
    float clampCallingRange(float callingRange) const;
    float adjustCallForRaises(const CurrentHandContext& context, float callingRange) const;
    float adjustCallForRaiserStats(float callingRange, const PreflopStatistics& raiserStats, int nbRaises,
                                   int nbPlayers, PlayerPosition myPosition, int nbRunningPlayers) const;
    float adjustCallForNoStats(float callingRange, int nbRaises) const;
    float adjustCallForBigBet(float callingRange, int potOdd, int myCash, int highestSet, int mySet,
                              int smallBlind) const;
    bool shouldAdjustCallForLooseRaiser(const CurrentHandContext& context, int nbCalls, int nbRaises) const;
    bool shouldCallForGoodOdds(int potOdd, int myM, PlayerPosition myPosition) const;
    bool shouldCallForAllIn(const CurrentHandContext& context, int potOdd, int nbRaises) const;

    // Raising range helper methods
    float adjustRaiseForLimpers(float raisingRange) const;
    float adjustRaiseForRaiser(const CurrentHandContext& context, float raisingRange) const;
    float adjustRaiseForRaiserStats(const PreflopStatistics& raiserStats, float raisingRange, int nbRaises,
                                    int nbPlayers) const;
    float adjustRaiseForNoRaiserStats(float raisingRange, int nbRaises) const;
    float adjustRaiseForNoRaiser(const CurrentHandContext& context, float raisingRange, bool canBluff,
                                 bool deterministic = false) const;
    float adjustRaiseForStack(const CurrentHandContext& context, float raisingRange) const;
    float adjustRaiseForBigBet(float raisingRange, int potOdd, int myCash, int highestSet, int mySet,
                               int smallBlind) const;
    float clampRaiseRange(float raisingRange) const;
};

} // namespace pkt::core::player