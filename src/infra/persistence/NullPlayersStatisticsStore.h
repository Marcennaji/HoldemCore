// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/persistence/PlayersStatisticsStore.h"
#include "core/engine/EngineDefs.h"
#include "core/player/PlayerStatistics.h"

#include <array>

namespace pkt::infra
{

/**
 * @brief Null object implementation of PlayersStatisticsStore interface.
 * 
 * Provides a no-operation implementation of the statistics store interface
 * for testing scenarios or when persistent statistics storage is not needed.
 * Follows the Null Object pattern to avoid null pointer checks.
 */
class NullPlayersStatisticsStore : public pkt::core::PlayersStatisticsStore
{
  public:
    NullPlayersStatisticsStore()
        : m_nullPlayerStatistics{pkt::core::player::PlayerStatistics()} // Initialize with default PlayerStatistics
    {
        // Ensure all PlayerStatistics are reset to zero
        for (auto& stats : m_nullPlayerStatistics)
        {
            stats.reset();
        }
    }
    virtual ~NullPlayersStatisticsStore() = default;

    virtual std::array<pkt::core::player::PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1>
    loadPlayerStatistics(const std::string& playerName) override
    {
        return m_nullPlayerStatistics;
    }

    virtual void savePlayersStatistics(pkt::core::player::PlayerList) override {}

  private:
    std::array<pkt::core::player::PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1> m_nullPlayerStatistics;
};
} //  namespace pkt::core
