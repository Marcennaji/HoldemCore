// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/player/typedefs.h>
#include <core/services/GlobalServices.h>
#include "core/engine/EngineDefs.h"
#include "core/engine/model/GameState.h"
#include "core/engine/model/PlayerStatistics.h"

#include <filesystem>
#include <string>

#define SQL_LOG_FILE "pokerTraining.db"

struct sqlite3;
struct sqlite3_stmt;

namespace pkt::infra
{

using core::ILogger;
using core::PlayerStatistics;
using core::player::PlayerList;

class SqliteLogStore
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

    std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1> getPlayerStatistics(const std::string& playerName);

  private:
    void createDatabase();
    void InitializeStrategyStatistics(const std::string playerName, const int nbPlayers);
    void exec_transaction();
    int getIntegerValue(const std::string playerName, const std::string tableName, const std::string attributeName);
    void createRankingTable();
    void createUnplausibleHandsTable();

    bool validateLogDirectory(std::filesystem::path& sqliteLogFileName) const;
    bool openDatabase(const std::filesystem::path& sqliteLogFileName, sqlite3*& db) const;
    bool prepareStatement(sqlite3* db, const std::string& sql_select, sqlite3_stmt*& stmt) const;
    void processQueryResults(sqlite3_stmt* stmt,
                             std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1>& playerStatistics) const;
    void updatePlayerStatistics(PlayerStatistics& stats, const std::string& columnName, sqlite3_stmt* stmt,
                                int nCol) const;

    sqlite3* mySqliteLogDb;
    std::filesystem::path mySqliteLogFileName;
    int uniqueGameID;
    int currentHandID;
    std::string sql;
    std::string myLogDir;
};
} // namespace pkt::infra
