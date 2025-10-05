// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/persistence/PlayersStatisticsStore.h"
#include <memory>

namespace pkt::core
{

/**
 * @brief Interface for components that need player statistics persistence
 * 
 * This replaces heavy ServiceContainer dependency with a focused interface
 * following Interface Segregation Principle.
 */
class HasPlayersStatisticsStore
{
  public:
    virtual ~HasPlayersStatisticsStore() = default;
    virtual PlayersStatisticsStore& playersStatisticsStore() = 0;
};

/**
 * @brief Simple wrapper around a PlayersStatisticsStore instance
 */
class PlayersStatisticsStoreService : public HasPlayersStatisticsStore
{
  public:
    explicit PlayersStatisticsStoreService(std::shared_ptr<PlayersStatisticsStore> store)
        : m_store(store) {}
    
    PlayersStatisticsStore& playersStatisticsStore() override
    {
        return *m_store;
    }

  private:
    std::shared_ptr<PlayersStatisticsStore> m_store;
};

} // namespace pkt::core