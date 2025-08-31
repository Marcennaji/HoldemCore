// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "PlayersStatisticsStore.h"
#include "core/engine/EngineDefs.h"
#include "core/player/PlayerStatistics.h"

#include <array>

namespace pkt::core
{

class NullPlayersStatisticsStore : public PlayersStatisticsStore
{
  public:
    NullPlayersStatisticsStore()
        : myNullPlayerStatistics{pkt::core::player::PlayerStatistics()} // Initialize with default PlayerStatistics
    {
        // Ensure all PlayerStatistics are reset to zero
        for (auto& stats : myNullPlayerStatistics)
        {
            stats.reset();
        }
    }
    virtual ~NullPlayersStatisticsStore() = default;

    virtual std::array<pkt::core::player::PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1>
    getPlayerStatistics(const std::string& playerName) override
    {
        return myNullPlayerStatistics;
    }

    virtual void updatePlayersStatistics(pkt::core::player::PlayerList) override {}

  private:
    std::array<pkt::core::player::PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1> myNullPlayerStatistics;
};
} //  namespace pkt::core
