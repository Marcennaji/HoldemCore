#pragma once

#include <core/engine/EngineDefs.h>
#include <core/engine/model/PlayerStatistics.h>
#include <memory>
#include <string>
#include <vector>

struct PostFlopState;

namespace pkt::core
{

class IHand;
class IPlayersStatisticsStore;

struct FlopStatistics;
struct TurnStatistics;
struct RiverStatistics;

class RangeManager
{
  public:
    RangeManager(int playerId, IPlayersStatisticsStore* statsStore);

    void setHand(IHand* hand) { myHand = hand; }
    void setEstimatedRange(const std::string& range);
    std::string getEstimatedRange() const;
    std::vector<std::string> getRangeAtomicValues(std::string range, const bool returnRange = false) const;
    void computeEstimatedPreflopRange(Player& opponent, int nbPlayers, int lastRaiserID, int preflopRaises,
                                      const PreflopStatistics& lastRaiserStats, bool lastRaiserIsInVeryLooseMode);
    int getStandardRaisingRange(int nbPlayers) const;
    int getStandardCallingRange(int nbPlayers) const;

  private:
    std::string computeEstimatedPreflopRangeFromLastRaiser(const Player& opponent,
                                                           const PreflopStatistics& opponentStats,
                                                           const PreflopStatistics& previousRaiserStats) const;
    std::string computeEstimatedPreflopRangeFromCaller(Player& opponent, PreflopStatistics& callerStats,
                                                       const PreflopStatistics& lastRaiserStats,
                                                       bool lastRaiserIsInVeryLooseMode) const;
    char incrementCardValue(char c) const;
    std::string getFilledRange(std::vector<std::string>& ranges, std::vector<float>& rangesValues,
                               const float rangeMax) const;

    std::string myEstimatedRange;
    int myPlayerId;
    IHand* myHand;
    IPlayersStatisticsStore* myStatsStore;
};
} // namespace pkt::core