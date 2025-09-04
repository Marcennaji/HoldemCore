// File: core/services/GlobalServices.cpp

#include "GlobalServices.h"
#include "DefaultRandomizer.h"
#include "core/interfaces/NullHandEvaluationEngine.h"
#include "core/interfaces/NullLogger.h"
#include "core/interfaces/persistence/NullPlayersStatisticsStore.h"

namespace pkt::core
{

GlobalServices::GlobalServices()
    : myLogger(std::make_unique<NullLogger>()),
      myPlayersStatisticsStore(std::make_unique<NullPlayersStatisticsStore>()),
      myRandomizer(std::make_unique<DefaultRandomizer>()),
      myHandEvaluationEngine(std::make_unique<NullHandEvaluationEngine>())
{
}

} // namespace pkt::core
