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
    void setLogger(std::unique_ptr<Logger> logger) { m_logger = std::move(logger); }
    void setPlayersStatisticsStore(std::unique_ptr<PlayersStatisticsStore> store)
    {
        m_playersStatisticsStore = std::move(store);
    }
    void setHandEvaluationEngine(std::unique_ptr<HandEvaluationEngine> engine)
    {
        m_handEvaluationEngine = std::move(engine);
    }
    void setRandomizer(std::unique_ptr<Randomizer> randomizer) { m_randomizer = std::move(randomizer); }

  private:
    std::unique_ptr<Logger> m_logger;
    std::unique_ptr<PlayersStatisticsStore> m_playersStatisticsStore;
    std::unique_ptr<HandEvaluationEngine> m_handEvaluationEngine;
    std::unique_ptr<Randomizer> m_randomizer;
};

} // namespace pkt::core