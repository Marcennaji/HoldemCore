// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/player/typedefs.h>
#include "SqliteDb.h"
#include "core/engine/EngineDefs.h"
#include "core/interfaces/persistence/PlayersStatisticsStore.h"
#include "core/player/PlayerStatistics.h"

#include <filesystem>
#include <string>

#define SQL_LOG_FILE "pokerTraining.db"

struct sqlite3;
struct sqlite3_stmt;

namespace pkt::infra
{

using core::player::PlayerList;
using core::player::PlayerStatistics;

class SqlitePlayersStatisticsStore : public pkt::core::PlayersStatisticsStore
{

  public:
    SqlitePlayersStatisticsStore(std::shared_ptr<SqliteDb> db);

    ~SqlitePlayersStatisticsStore();

    void updatePlayersStatistics(PlayerList seatsList);

    std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1> getPlayerStatistics(const std::string& playerName);

  private:
    void initializeStrategyStatistics(const std::string& playerName, const int nbPlayers);
    void updatePlayerStatistics(PlayerStatistics& stats, const std::string& columnName, sqlite3_stmt* stmt,
                                int nCol) const;

    std::shared_ptr<SqliteDb> myDb;
    std::string mySql;
};
} // namespace pkt::infra
