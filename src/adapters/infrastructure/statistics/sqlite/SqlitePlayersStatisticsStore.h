// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/player/typedefs.h>
#include "SqliteDb.h"
#include "core/engine/EngineDefs.h"
#include "core/ports/PlayersStatisticsStore.h"
#include "core/player/PlayerStatistics.h"

#include <filesystem>
#include <string>

namespace pkt::infra
{

using core::player::PlayerList;
using core::player::PlayerStatistics;

/**
 * @brief SQLite database implementation for storing player statistics.
 * 
 * This class provides persistent storage of player performance data using SQLite.
 * It tracks wins, losses, hands played, and other statistical information
 * for analysis and long-term player performance monitoring.
 */
class SqlitePlayersStatisticsStore : public pkt::core::PlayersStatisticsStore
{

  public:
    SqlitePlayersStatisticsStore(std::unique_ptr<SqliteDb> db);

    ~SqlitePlayersStatisticsStore();

    void savePlayersStatistics(PlayerList seatsList) override;
    std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1>
    loadPlayerStatistics(const std::string& playerName) override;

  private:
    void initializeStrategyStatistics(const std::string& playerName, const int nbPlayers);
    std::unique_ptr<SqliteDb> m_db;
    std::string m_sql;
};
} // namespace pkt::infra
