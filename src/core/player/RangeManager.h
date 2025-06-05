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

  private:
    std::string myEstimatedRange;
    int myPlayerId;
    IHand* myHand;
    IPlayersStatisticsStore* myStatsStore;
};
} // namespace pkt::core