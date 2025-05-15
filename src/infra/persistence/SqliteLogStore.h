/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU Affero General Public License as            *
 * published by the Free Software Foundation, either version 3 of the        *
 * License, or (at your option) any later version.                           *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU Affero General Public License for more details.                       *
 *                                                                           *
 * You should have received a copy of the GNU Affero General Public License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#pragma once

#define SQLLITE_COMPLETE_LOG

#include <core/interfaces/persistence/IHandAuditStore.h>
#include <core/interfaces/persistence/IPlayersStatisticsStore.h>
#include <core/interfaces/persistence/IRankingStore.h>
#include "core/engine/EngineDefs.h"
#include "core/engine/model/GameState.h"
#include "core/engine/model/PlayerStatistics.h"

#include <filesystem>
#include <string>

#define SQL_LOG_FILE "pokerTraining-log-v0.9.pdb"

struct sqlite3;

class SqliteLogStore : public IRankingStore, public IHandAuditStore, public IPlayersStatisticsStore
{

  public:
    SqliteLogStore(const std::string& logDir);

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
};
