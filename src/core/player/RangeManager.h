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
    RangeManager(int playerId, IHand* hand, IPlayersStatisticsStore* statsStore);

    void setEstimatedRange(const std::string& range);
    std::string getEstimatedRange() const;
    std::vector<std::string> getRangeAtomicValues(std::string range, const bool returnRange = false) const;
    void computeEstimatedPreflopRange(Player& opponent, int nbPlayers, int lastRaiserID, int preflopRaises,
                                      const PreflopStatistics& lastRaiserStats);
    int getStandardRaisingRange(int nbPlayers) const;
    int getStandardCallingRange(int nbPlayers) const;

  private:
    std::string computeEstimatedPreflopRangeFromLastRaiser(const Player& opponent,
                                                           const PreflopStatistics& opponentStats,
                                                           const PreflopStatistics& previousRaiserStats) const;
    char incrementCardValue(char c) const;

    std::string myEstimatedRange;
    int myPlayerId;
    IHand* myHand;
    IPlayersStatisticsStore* myStatsStore;
};
} // namespace pkt::core