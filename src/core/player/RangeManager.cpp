#include "RangeManager.h"

#include <algorithm>
#include <core/engine/model/PlayerStatistics.h>
#include <core/interfaces/IHand.h>
#include <core/interfaces/persistence/IPlayersStatisticsStore.h>
#include <sstream>

namespace pkt::core
{

RangeManager::RangeManager(int playerId, IHand* hand, IPlayersStatisticsStore* statsStore)
    : myPlayerId(playerId), myHand(hand), myStatsStore(statsStore)
{
}

void RangeManager::setEstimatedRange(const std::string& range)
{
    myEstimatedRange = range;
}

std::string RangeManager::getEstimatedRange() const
{
    return myEstimatedRange;
}

} // namespace pkt::core