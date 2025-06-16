/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the MIT License                                     *


 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * MIT License for more details.                                             *
 *                                                                           *
 * You should have received a copy of the MIT License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#pragma once

#define SQLLITE_COMPLETE_LOG

#include <core/interfaces/persistence/IHandAuditStore.h>
#include <core/interfaces/persistence/IPlayersStatisticsStore.h>
#include <core/interfaces/persistence/IRankingStore.h>
#include <core/player/typedefs.h>
#include "core/engine/EngineDefs.h"
#include "core/engine/model/GameState.h"
#include "core/engine/model/PlayerStatistics.h"
#include "core/interfaces/ILogger.h"

#include <filesystem>
#include <string>

#define SQL_LOG_FILE "pokerTraining.db"

struct sqlite3;
namespace pkt::infra
{

using core::ILogger;
using core::PlayerStatistics;
using core::player::PlayerList;

class SqliteLogStore : public core::IRankingStore, public core::IHandAuditStore, public core::IPlayersStatisticsStore
{

  public:
    SqliteLogStore(const std::string& logDir, ILogger*);

    ~SqliteLogStore();

    void init();
    void updateRankingGameLosers(PlayerList activePlayerList);
    void updateRankingGameWinner(PlayerList activePlayerList);
    void updateRankingPlayedGames(PlayerList activePlayerList);
    void updateUnplausibleHand(const std::string card1, const std::string card2, const bool human,
                               const char bettingRound, const int nbPlayers);
    void updatePlayersStatistics(PlayerList activePlayerList);
    void InitializePlayersStatistics(const std::string playerName, const int nbPlayers);
    std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1> getPlayerStatistics(const std::string& playerName);
    void createDatabase();

    std::string getSqliteLogFileName() { return mySqliteLogFileName.string(); }

  private:
    void exec_transaction();
    int getIntegerValue(const std::string playerName, const std::string tableName, const std::string attributeName);
    void createRankingTable();
    void createUnplausibleHandsTable();

    sqlite3* mySqliteLogDb;
    std::filesystem::path mySqliteLogFileName;
    int uniqueGameID;
    int currentHandID;
    std::string sql;
    std::string myLogDir;
    ILogger* myLogger;
};
} // namespace pkt::infra
