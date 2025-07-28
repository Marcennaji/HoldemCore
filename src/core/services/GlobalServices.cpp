// File: core/services/GlobalServices.cpp

#include "GlobalServices.h"
#include "DefaultRandomizer.h"
#include "core/interfaces/NullLogger.h"
#include "core/interfaces/persistence/NullHandAuditStore.h"
#include "core/interfaces/persistence/NullPlayersStatisticsStore.h"
#include "core/interfaces/persistence/NullRankingStore.h"

namespace pkt::core
{

GlobalServices::GlobalServices()
    : myLogger(std::make_shared<NullLogger>()), myRankingStore(std::make_shared<NullRankingStore>()),
      myHandAuditStore(std::make_shared<NullHandAuditStore>()),
      myPlayersStatisticsStore(std::make_shared<NullPlayersStatisticsStore>()),
      myRandomizer(std::make_shared<DefaultRandomizer>())
{
}

} // namespace pkt::core
