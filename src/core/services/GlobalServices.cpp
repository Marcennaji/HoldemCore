// File: core/services/GlobalServices.cpp

#include "GlobalServices.h"
#include "DefaultRandomizer.h"
#include "core/interfaces/NullLogger.h"
#include "core/interfaces/persistence/NullPlayersStatisticsStore.h"

namespace pkt::core
{

GlobalServices::GlobalServices()
    : myLogger(std::make_shared<NullLogger>()),
      myPlayersStatisticsStore(std::make_shared<NullPlayersStatisticsStore>()),
      myRandomizer(std::make_shared<DefaultRandomizer>())
{
}

} // namespace pkt::core
