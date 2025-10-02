// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/HandEvaluationEngine.h"
#include "core/interfaces/Logger.h"
#include "core/interfaces/Randomizer.h"
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
    virtual Logger& logger() = 0;
    virtual PlayersStatisticsStore& playersStatisticsStore() = 0;
    virtual HandEvaluationEngine& handEvaluationEngine() = 0;
    virtual Randomizer& randomizer() = 0;
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

    Logger& logger() override;
    PlayersStatisticsStore& playersStatisticsStore() override;
    HandEvaluationEngine& handEvaluationEngine() override;
    Randomizer& randomizer() override;

    // Configuration methods for setting up services
    void setLogger(std::unique_ptr<Logger> logger) { myLogger = std::move(logger); }
    void setPlayersStatisticsStore(std::unique_ptr<PlayersStatisticsStore> store)
    {
        myPlayersStatisticsStore = std::move(store);
    }
    void setHandEvaluationEngine(std::unique_ptr<HandEvaluationEngine> engine)
    {
        myHandEvaluationEngine = std::move(engine);
    }
    void setRandomizer(std::unique_ptr<Randomizer> randomizer) { myRandomizer = std::move(randomizer); }

  private:
    std::unique_ptr<Logger> myLogger;
    std::unique_ptr<PlayersStatisticsStore> myPlayersStatisticsStore;
    std::unique_ptr<HandEvaluationEngine> myHandEvaluationEngine;
    std::unique_ptr<Randomizer> myRandomizer;
};

} // namespace pkt::core