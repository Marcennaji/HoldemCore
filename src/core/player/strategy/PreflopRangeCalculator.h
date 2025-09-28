#pragma once

#include <core/engine/model/PlayerPosition.h>
#include <core/player/PlayerStatistics.h>
#include <core/services/ServiceContainer.h>

#include <vector>

namespace pkt::core::player
{
struct CurrentHandContext;

class PreflopRangeCalculator
{
  public:
    PreflopRangeCalculator() = default;
    explicit PreflopRangeCalculator(std::shared_ptr<pkt::core::ServiceContainer> serviceContainer);
    // Allow wiring services after default construction when owned by BotStrategyBase
    void setServices(std::shared_ptr<pkt::core::ServiceContainer> serviceContainer) { myServices = serviceContainer; }
    float calculatePreflopCallingRange(const CurrentHandContext& ctx) const;
    float calculatePreflopRaisingRange(const CurrentHandContext& ctx) const;

    int getRange(PlayerPosition p, int nbPlayers) const;
    void initializeRanges(const int utgHeadsUpRange, const int utgFullTableRange);

  private:
    // Vector index is player position, value is range %
    std::vector<int> myUtgStartingRange;
    std::vector<int> myUtgPlusOneStartingRange;
    std::vector<int> myUtgPlusTwoStartingRange;
    std::vector<int> myMiddleStartingRange;
    std::vector<int> myMiddlePlusOneStartingRange;
    std::vector<int> myLateStartingRange;
    std::vector<int> myCutoffStartingRange;
    std::vector<int> myButtonStartingRange;
    std::vector<int> mySmallBlindStartingRange;
    std::vector<int> myBigBlindStartingRange;

    // calling range helper methods
    float adjustCallForLimpers(float callingRange) const;
    float clampCallingRange(float callingRange) const;
    float adjustCallForRaises(const CurrentHandContext& ctx, float callingRange) const;
    float adjustCallForRaiserStats(float callingRange, const PreflopStatistics& raiserStats, int nbRaises,
                                   int nbPlayers, PlayerPosition myPosition, int nbActingPlayers) const;
    float adjustCallForNoStats(float callingRange, int nbRaises) const;
    float adjustCallForBigBet(float callingRange, int potOdd, int myCash, int highestBetAmount, int myTotalBetAmount,
                              int smallBlind) const;
    bool shouldAdjustCallForLooseRaiser(const CurrentHandContext& ctx, int nbCalls, int nbRaises) const;
    bool couldCallForGoodOdds(int potOdd, int myM, PlayerPosition myPosition) const;
    bool couldCallForAllIn(const CurrentHandContext& ctx, int potOdd, int nbRaises) const;

    // Raising range helper methods
    float adjustRaiseForLimpers(float raisingRange) const;
    float adjustRaiseForRaiser(const CurrentHandContext& ctx, float raisingRange) const;
    float adjustRaiseForRaiserStats(const PreflopStatistics& raiserStats, float raisingRange, int nbRaises,
                                    int nbPlayers) const;
    float adjustRaiseForNoRaiserStats(float raisingRange, int nbRaises) const;
    float adjustRaiseForNoRaiser(const CurrentHandContext& ctx, float raisingRange, bool canBluff) const;
    float adjustRaiseForStack(const CurrentHandContext& ctx, float raisingRange) const;
    float adjustRaiseForBigBet(float raisingRange, int potOdd, int myCash, int highestBetAmount, int myTotalBetAmount,
                               int smallBlind) const;
    float clampRaiseRange(float raisingRange) const;

  private:
    void ensureServicesInitialized() const;
    mutable std::shared_ptr<pkt::core::ServiceContainer> myServices; // Injected service container
};

} // namespace pkt::core::player