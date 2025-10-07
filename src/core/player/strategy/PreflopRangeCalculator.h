#pragma once

#include <memory>
#include <core/engine/model/PlayerPosition.h>
#include <core/player/PlayerStatistics.h>

#include <core/interfaces/Logger.h>
#include <core/interfaces/Randomizer.h>

#include <vector>

namespace pkt::core::player
{
struct CurrentHandContext;

class PreflopRangeCalculator
{
  public:
    PreflopRangeCalculator(pkt::core::Logger& logger, pkt::core::Randomizer& randomizer);
    
    float calculatePreflopCallingRange(const CurrentHandContext& ctx) const;
    float calculatePreflopRaisingRange(const CurrentHandContext& ctx) const;

    int getRange(PlayerPosition p, int nbPlayers) const;
    void initializeRanges(const int utgHeadsUpRange, const int utgFullTableRange);

  private:
    // Vector index is player position, value is range %
    std::vector<int> m_utgStartingRange;
    std::vector<int> m_utgPlusOneStartingRange;
    std::vector<int> m_utgPlusTwoStartingRange;
    std::vector<int> m_middleStartingRange;
    std::vector<int> m_middlePlusOneStartingRange;
    std::vector<int> m_lateStartingRange;
    std::vector<int> m_cutoffStartingRange;
    std::vector<int> m_buttonStartingRange;
    std::vector<int> m_smallBlindStartingRange;
    std::vector<int> m_bigBlindStartingRange;

    // calling range helper methods
    float adjustCallForLimpers(float callingRange) const;
    float clampCallingRange(float callingRange) const;
    float adjustCallForRaises(const CurrentHandContext& ctx, float callingRange) const;
    float adjustCallForRaiserStats(float callingRange, const PreflopStatistics& raiserStats, int nbRaises,
                                   int nbPlayers, PlayerPosition m_position, int nbActingPlayers) const;
    float adjustCallForNoStats(float callingRange, int nbRaises) const;
    float adjustCallForBigBet(float callingRange, int potOdd, int m_cash, int highestBetAmount, int m_totalBetAmount,
                              int smallBlind) const;
    bool shouldAdjustCallForLooseRaiser(const CurrentHandContext& ctx, int nbCalls, int nbRaises) const;
    bool couldCallForGoodOdds(int potOdd, int m_m, PlayerPosition m_position) const;
    bool couldCallForAllIn(const CurrentHandContext& ctx, int potOdd, int nbRaises) const;

    // Raising range helper methods
    float adjustRaiseForLimpers(float raisingRange) const;
    float adjustRaiseForRaiser(const CurrentHandContext& ctx, float raisingRange) const;
    float adjustRaiseForRaiserStats(const PreflopStatistics& raiserStats, float raisingRange, int nbRaises,
                                    int nbPlayers) const;
    float adjustRaiseForNoRaiserStats(float raisingRange, int nbRaises) const;
    float adjustRaiseForNoRaiser(const CurrentHandContext& ctx, float raisingRange, bool canBluff) const;
    float adjustRaiseForStack(const CurrentHandContext& ctx, float raisingRange) const;
    float adjustRaiseForBigBet(float raisingRange, int potOdd, int m_cash, int highestBetAmount, int m_totalBetAmount,
                               int smallBlind) const;
    float clampRaiseRange(float raisingRange) const;

  private:
    
    // ISP-compliant service access helpers
    pkt::core::Logger& getLogger() const;
    pkt::core::Randomizer& getRandomizer() const;
    
    // ISP-compliant focused service interfaces
    pkt::core::Logger* m_logger;
    pkt::core::Randomizer* m_randomizer;
};

} // namespace pkt::core::player