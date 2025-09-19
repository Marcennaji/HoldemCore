#pragma once

#include "core/interfaces/HandEvaluationEngine.h"
#include "core/interfaces/ILogger.h"
#include "core/interfaces/IRandomizer.h"
#include "core/interfaces/persistence/PlayersStatisticsStore.h"

#include <memory>

namespace pkt::core
{

class GlobalServices
{
  public:
    static GlobalServices& instance()
    {
        static GlobalServices inst;
        return inst;
    }

    ILogger& logger() { return *myLogger; }
    PlayersStatisticsStore& playersStatisticsStore() { return *myPlayersStatisticsStore; }
    HandEvaluationEngine& handEvaluationEngine() { return *myHandEvaluationEngine; }
    IRandomizer& randomizer() { return *myRandomizer; }

    void setLogger(std::unique_ptr<ILogger> logger) { myLogger = std::move(logger); }
    void setPlayersStatisticsStore(std::unique_ptr<PlayersStatisticsStore> store)
    {
        myPlayersStatisticsStore = std::move(store);
    }
    void setHandEvaluationEngine(std::unique_ptr<HandEvaluationEngine> engine)
    {
        myHandEvaluationEngine = std::move(engine);
    }
    void setRandomizer(std::unique_ptr<IRandomizer> rand) { myRandomizer = std::move(rand); }

  private:
    GlobalServices();

    std::unique_ptr<ILogger> myLogger;
    std::unique_ptr<PlayersStatisticsStore> myPlayersStatisticsStore;
    std::unique_ptr<HandEvaluationEngine> myHandEvaluationEngine;
    std::unique_ptr<IRandomizer> myRandomizer;
};

} // namespace pkt::core