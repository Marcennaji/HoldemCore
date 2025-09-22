// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/HandEvaluationEngine.h"
#include "core/interfaces/ILogger.h"
#include "core/interfaces/IRandomizer.h"
#include "core/interfaces/persistence/PlayersStatisticsStore.h"

#include <memory>

namespace pkt::core
{

/**
 * @brief Service container interface for dependency injection
 *
 * Provides a clean interface for accessing core services without
 * directly depending on the dependency injection container.
 */
class ServiceContainer
{
  public:
    virtual ~ServiceContainer() = default;

    // Core service accessors
    virtual ILogger& logger() = 0;
    virtual PlayersStatisticsStore& playersStatisticsStore() = 0;
    virtual HandEvaluationEngine& handEvaluationEngine() = 0;
    virtual IRandomizer& randomizer() = 0;
};

/**
 * @brief Default implementation that creates its own service instances
 *
 * Provides a self-contained service container with default implementations.
 */
class AppServiceContainer : public ServiceContainer
{
  public:
    AppServiceContainer();
    ~AppServiceContainer() override = default;

    ILogger& logger() override;
    PlayersStatisticsStore& playersStatisticsStore() override;
    HandEvaluationEngine& handEvaluationEngine() override;
    IRandomizer& randomizer() override;

    // Configuration methods for setting up services
    void setLogger(std::unique_ptr<ILogger> logger) { myLogger = std::move(logger); }
    void setPlayersStatisticsStore(std::unique_ptr<PlayersStatisticsStore> store)
    {
        myPlayersStatisticsStore = std::move(store);
    }
    void setHandEvaluationEngine(std::unique_ptr<HandEvaluationEngine> engine)
    {
        myHandEvaluationEngine = std::move(engine);
    }
    void setRandomizer(std::unique_ptr<IRandomizer> randomizer) { myRandomizer = std::move(randomizer); }

  private:
    std::unique_ptr<ILogger> myLogger;
    std::unique_ptr<PlayersStatisticsStore> myPlayersStatisticsStore;
    std::unique_ptr<HandEvaluationEngine> myHandEvaluationEngine;
    std::unique_ptr<IRandomizer> myRandomizer;
};

} // namespace pkt::core