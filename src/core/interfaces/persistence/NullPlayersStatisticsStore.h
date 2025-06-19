// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "IPlayersStatisticsStore.h"
#include "core/engine/EngineDefs.h"
#include "core/engine/model/PlayerStatistics.h"

#include <array>

namespace pkt::core
{

class NullPlayersStatisticsStore : public IPlayersStatisticsStore
{
  public:
    NullPlayersStatisticsStore()
        : m_nullPlayerStatistics{PlayerStatistics()} // Initialize with default PlayerStatistics
    {
        // Ensure all PlayerStatistics are reset to zero
        for (auto& stats : m_nullPlayerStatistics)
        {
            stats.reset();
        }
    }
    virtual ~NullPlayersStatisticsStore() = default;

    virtual std::array<pkt::core::PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1>
    getPlayerStatistics(const std::string& playerName) override
    {
        return m_nullPlayerStatistics;
    }

    virtual void updatePlayersStatistics(pkt::core::player::PlayerList activePlayerList) override {}

  private:
    std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1> m_nullPlayerStatistics;
};
} //  namespace pkt::core
