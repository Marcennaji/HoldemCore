// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/Logger.h"
#include "core/interfaces/Randomizer.h"
#include "core/interfaces/HandEvaluationEngine.h"
#include "core/interfaces/persistence/PlayersStatisticsStore.h"
#include "core/services/ServiceContainer.h"
#include <memory>

namespace pkt::core
{

/**
 * @brief Helper to create focused service dependencies from ServiceContainer
 * 
 * This adapter makes it easy to migrate from ServiceContainer to focused dependencies
 * without changing all the creation code at once.
 */
class ServiceAdapter
{
  public:
    explicit ServiceAdapter(std::shared_ptr<ServiceContainer> services) : m_services(services) {}
    
    std::shared_ptr<Logger> createLoggerService() const {
        return std::shared_ptr<Logger>(&m_services->logger(), [](Logger*){});
    }
    
    std::shared_ptr<Randomizer> createRandomizerService() const {
        return std::shared_ptr<Randomizer>(&m_services->randomizer(), [](Randomizer*){});
    }
    
    std::shared_ptr<HandEvaluationEngine> createHandEvaluationEngineService() const {
        return std::shared_ptr<HandEvaluationEngine>(&m_services->handEvaluationEngine(), [](HandEvaluationEngine*){});
    }
    
    std::shared_ptr<PlayersStatisticsStore> createPlayersStatisticsStoreService() const {
        return std::shared_ptr<PlayersStatisticsStore>(&m_services->playersStatisticsStore(), [](PlayersStatisticsStore*){});
    }
    
  private:
    std::shared_ptr<ServiceContainer> m_services;
};

} // namespace pkt::core