#pragma once

#include "core/interfaces/IHandEvaluationEngine.h"
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

    // Getters
    std::shared_ptr<ILogger> logger() const { return myLogger; }
    std::shared_ptr<PlayersStatisticsStore> playersStatisticsStore() const { return myPlayersStatisticsStore; }
    std::shared_ptr<IHandEvaluationEngine> handEvaluationEngine() const { return myHandEvaluationEngine; }
    std::shared_ptr<IRandomizer> randomizer() const { return myRandomizer; }

    // Setters
    void setLogger(std::shared_ptr<ILogger> logger) { myLogger = std::move(logger); }
    void setPlayersStatisticsStore(std::shared_ptr<PlayersStatisticsStore> store)
    {
        myPlayersStatisticsStore = std::move(store);
    }
    void setHandEvaluationEngine(std::shared_ptr<IHandEvaluationEngine> engine)
    {
        myHandEvaluationEngine = std::move(engine);
    }
    void setRandomizer(std::shared_ptr<IRandomizer> rand) { myRandomizer = std::move(rand); }

  private:
    GlobalServices();

    std::shared_ptr<ILogger> myLogger;
    std::shared_ptr<PlayersStatisticsStore> myPlayersStatisticsStore;
    std::shared_ptr<IHandEvaluationEngine> myHandEvaluationEngine;
    std::shared_ptr<IRandomizer> myRandomizer;
};

} // namespace pkt::core