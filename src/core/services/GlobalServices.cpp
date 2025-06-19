// File: core/services/GlobalServices.cpp

#include "GlobalServices.h"
#include "core/interfaces/NullLogger.h"
#include "core/interfaces/persistence/NullHandAuditStore.h"
#include "core/interfaces/persistence/NullPlayersStatisticsStore.h"
#include "core/interfaces/persistence/NullRankingStore.h"

namespace pkt::core
{

GlobalServices::GlobalServices()
    : logger_(std::make_shared<NullLogger>()), rankingStore_(std::make_shared<NullRankingStore>()),
      handAuditStore_(std::make_shared<NullHandAuditStore>()),
      playersStatisticsStore_(std::make_shared<NullPlayersStatisticsStore>())
{
}

} // namespace pkt::core
