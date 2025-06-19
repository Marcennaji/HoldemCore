// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "SqliteLogStore.h"
#include <core/player/typedefs.h>
#include "core/engine/CardsValue.h"
#include "core/engine/EngineDefs.h"
#include "core/engine/model/PlayerStatistics.h"
#include "core/interfaces/NullLogger.h"
#include "core/player/Player.h"
#include "core/player/strategy/LooseAggressiveBotStrategy.h"
#include "core/player/strategy/ManiacBotStrategy.h"
#include "core/player/strategy/TightAggressiveBotStrategy.h"
#include "core/player/strategy/UltraTightBotStrategy.h"

#include <third_party/sqlite3/sqlite3.h>

#include <array>
#include <sstream>
#include <sys/types.h>

namespace pkt::infra
{

using namespace std;
using namespace pkt::core;
using namespace pkt::core::player;

SqliteLogStore::SqliteLogStore(const std::string& logDir)
    : mySqliteLogDb(0), mySqliteLogFileName(""), myLogDir(logDir), uniqueGameID(0), currentHandID(0), sql("")
{
}

SqliteLogStore::~SqliteLogStore()
{

    sqlite3_close(mySqliteLogDb);
}

void SqliteLogStore::init()
{

    bool dirExists;

    dirExists = std::filesystem::is_directory(myLogDir);

    // check if logging path exist
    if (myLogDir != "" && dirExists)
    {

        mySqliteLogFileName.clear();
        mySqliteLogFileName /= myLogDir;
        mySqliteLogFileName /= string(SQL_LOG_FILE);

        bool databaseExists = false;
        if (FILE* file = fopen(mySqliteLogFileName.string().c_str(), "r"))
        {
            fclose(file);
            databaseExists = true;
        }
        else
        {

            GlobalServices::instance().logger()->info("warning : database does not exist, will be created");
        }

        // open sqlite-db
        sqlite3_open(mySqliteLogFileName.string().c_str(), &mySqliteLogDb);

        if (mySqliteLogDb != 0)
        {

            if (!databaseExists)
                createDatabase();
        }
    }
}

void SqliteLogStore::createDatabase()
{

    createRankingTable();
    createUnplausibleHandsTable();

    // create stats table
    sql += "CREATE TABLE  IF NOT EXISTS PlayersStatistics (";
    sql += "strategy_name VARCHAR NOT NULL";
    sql += ", nb_players SMALLINT NOT NULL";
    sql += ", pf_hands LARGEINT ";
    // preflop stats :
    sql += ", pf_folds LARGEINT ";
    sql += ", pf_limps LARGEINT ";
    sql += ", pf_checks LARGEINT ";
    sql += ", pf_calls LARGEINT ";
    sql += ", pf_raises LARGEINT ";
    sql += ", pf_3Bets LARGEINT ";
    sql += ", pf_call3Bets LARGEINT ";
    sql += ", pf_call3BetsOpportunities LARGEINT ";
    sql += ", pf_4Bets LARGEINT ";
    // flop stats :
    sql += ", f_hands LARGEINT ";
    sql += ", f_folds LARGEINT ";
    sql += ", f_checks LARGEINT ";
    sql += ", f_bets LARGEINT ";
    sql += ", f_calls LARGEINT ";
    sql += ", f_raises LARGEINT ";
    sql += ", f_3Bets LARGEINT ";
    sql += ", f_4Bets LARGEINT ";
    sql += ", f_continuationBets LARGEINT ";
    sql += ", f_continuationBetsOpportunities LARGEINT ";
    // turn stats :
    sql += ", t_hands LARGEINT ";
    sql += ", t_folds LARGEINT ";
    sql += ", t_checks LARGEINT ";
    sql += ", t_calls LARGEINT ";
    sql += ", t_bets LARGEINT ";
    sql += ", t_raises LARGEINT ";
    sql += ", t_3Bets LARGEINT ";
    sql += ", t_4Bets LARGEINT ";
    // river stats :
    sql += ", r_hands LARGEINT ";
    sql += ", r_folds LARGEINT ";
    sql += ", r_bets LARGEINT ";
    sql += ", r_checks LARGEINT ";
    sql += ", r_calls LARGEINT ";
    sql += ", r_raises LARGEINT ";
    sql += ", r_3Bets LARGEINT ";
    sql += ", r_4Bets LARGEINT ";

    sql += ", PRIMARY KEY(strategy_name, nb_players));";

    exec_transaction();

    auto looseAggressiveStrategy = LooseAggressiveBotStrategy();
    auto tightAggressiveStrategy = TightAggressiveBotStrategy();
    auto maniacStrategy = ManiacBotStrategy();
    auto ultraTightStrategy = UltraTightBotStrategy();

    for (int j = 2; j <= MAX_NUMBER_OF_PLAYERS; j++)
    {
        InitializeStrategyStatistics("You", j); // human player
        // initialize players statistics for all bot strategies
        InitializeStrategyStatistics(tightAggressiveStrategy.getStrategyName(), j);
        InitializeStrategyStatistics(looseAggressiveStrategy.getStrategyName(), j);
        InitializeStrategyStatistics(maniacStrategy.getStrategyName(), j);
        InitializeStrategyStatistics(ultraTightStrategy.getStrategyName(), j);
    }
}

void SqliteLogStore::InitializeStrategyStatistics(const string playerName, const int nbPlayers)
{

    sql += "INSERT OR REPLACE INTO PlayersStatistics (";
    sql += "strategy_name,nb_players";
    sql += ",pf_hands,pf_checks,pf_calls,pf_raises,pf_3Bets,pf_call3Bets,pf_call3BetsOpportunities,pf_4Bets,pf_folds,"
           "pf_limps";
    sql += ",f_hands,f_checks,f_bets,f_calls,f_raises,f_3Bets,f_4Bets,f_folds,f_continuationBets,f_"
           "continuationBetsOpportunities";
    sql += ",t_hands,t_checks,t_bets,t_calls,t_raises,t_3Bets,t_4Bets,t_folds";
    sql += ",r_hands,r_checks,r_bets,r_calls,r_raises,r_3Bets,r_4Bets,r_folds";
    sql += ") VALUES (";
    sql += "'";
    sql += playerName;
    sql += "',";
    sql += std::to_string(nbPlayers);
    sql += ",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);";

    exec_transaction();
}

void SqliteLogStore::updateRankingGameLosers(PlayerList activePlayerList)
{
    createRankingTable();

    int losers = 0;
    PlayerListConstIterator it_c;
    for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
    {
        if ((*it_c)->getCash() == 0)
        {
            sql += "INSERT OR IGNORE INTO Ranking VALUES ('" + (*it_c)->getStrategyName() + "', 0, 0, 0);";
            const int lostStack = getIntegerValue((*it_c)->getStrategyName(), "Ranking", "lost_stack") + 1;
            sql += "UPDATE Ranking SET lost_stack = " + std::to_string(lostStack);
            sql += " WHERE strategy_name = '" + (*it_c)->getStrategyName() + "';";
        }
    }

    exec_transaction();
}
void SqliteLogStore::updateRankingGameWinner(PlayerList activePlayerList)
{
    createRankingTable();

    int playersPositiveCashCounter = 0;
    PlayerListConstIterator it_c;
    for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
    {
        if ((*it_c)->getCash() > 0)
            playersPositiveCashCounter++;
    }
    if (playersPositiveCashCounter == 1)
    {
        for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
        {
            if ((*it_c)->getCash() > 0)
            {
                sql += "INSERT OR IGNORE INTO Ranking VALUES ('" + (*it_c)->getStrategyName() + "', 0, 0, 0);";
                const int wonGame = getIntegerValue((*it_c)->getStrategyName(), "Ranking", "won_game") + 1;
                sql += "UPDATE Ranking SET won_game = " + std::to_string(wonGame);
                sql += " WHERE strategy_name = '" + (*it_c)->getStrategyName() + "';";
            }
        }
    }

    exec_transaction();
}
void SqliteLogStore::updateRankingPlayedGames(PlayerList activePlayerList)
{
    createRankingTable();

    int losers = 0;
    PlayerListConstIterator it_c;
    for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
    {
        sql += "INSERT OR IGNORE INTO Ranking VALUES ('" + (*it_c)->getStrategyName() + "', 0, 0, 0);";
        const int playedGames = getIntegerValue((*it_c)->getStrategyName(), "Ranking", "played_games") + 1;
        sql += "UPDATE Ranking SET played_games = " + std::to_string(playedGames);
        sql += " WHERE strategy_name = '" + (*it_c)->getStrategyName() + "';";
    }

    exec_transaction();
}
int SqliteLogStore::getIntegerValue(const std::string playerName, const std::string tableName,
                                    const std::string attributeName)
{

    int result = 0;

    if (mySqliteLogDb != 0)
    {
        // sqlite-db is open

        char** result_Player = 0;
        int nRow_Player = 0;
        int nCol_Player = 0;
        char* errmsg = 0;

        // read seat
        string sql_select =
            "SELECT " + attributeName + " FROM " + tableName + " WHERE strategy_name = \"" + playerName + "\"";
        if (sqlite3_get_table(mySqliteLogDb, sql_select.c_str(), &result_Player, &nRow_Player, &nCol_Player, &errmsg) !=
            SQLITE_OK)
        {
            GlobalServices::instance().logger()->error("Error in statement: " + sql_select + "[" + errmsg + "].");
        }
        else
        {
            if (nRow_Player == 1)
            {
                result = atoi(result_Player[1]);
            }
            else
            {
                GlobalServices::instance().logger()->info("no data for player " + playerName + " in table " +
                                                          tableName);
                GlobalServices::instance().logger()->info("SQL query was :\n" + sql_select);
            }
        }
        sqlite3_free_table(result_Player);
    }
    return result;
}

void SqliteLogStore::createRankingTable()
{

    // create table if doesn't exist
    sql = "CREATE TABLE IF NOT EXISTS Ranking (";
    sql += "strategy_name VARCHAR NOT NULL";
    sql += ",lost_stack LARGEINT";
    sql += ",won_game LARGEINT";
    sql += ",played_games LARGEINT";
    sql += ", PRIMARY KEY(strategy_name));";
    exec_transaction();
}
void SqliteLogStore::updateUnplausibleHand(const std::string card1, const std::string card2, const bool human,
                                           const char bettingRound, const int nbPlayers)
{
    createUnplausibleHandsTable();

    std::stringstream hand;

    if (card1.at(1) == card2.at(1))
    {
        // suited hand
        if (CardsValue::CardStringOrdering[card1] > CardsValue::CardStringOrdering[card2])
            hand << card1 << card2;
        else if (CardsValue::CardStringOrdering[card1] < CardsValue::CardStringOrdering[card2])
            hand << card2 << card1;
    }
    else
    {
        // unsuited hand
        if (CardsValue::CardStringOrdering[card1] > CardsValue::CardStringOrdering[card2])
            hand << card1.at(0) << card2.at(0) << 'o';
        else if (CardsValue::CardStringOrdering[card1] < CardsValue::CardStringOrdering[card2])
            hand << card2.at(0) << card1.at(0) << 'o';
        else
            hand << card1.at(0) << card2.at(0);
    }

    int losers = 0;
    sql += "INSERT OR IGNORE INTO UnplausibleHands VALUES ('" + hand.str() + "','" + bettingRound + "'," +
           std::to_string(nbPlayers) + (human ? ", 1" : ", 0") + ", 0);";

    // get previous count value
    int previousCount = 0;

    if (mySqliteLogDb != 0)
    {

        char** result = 0;
        int nRow = 0;
        int nCol = 0;
        char* errmsg = 0;

        // read seat
        string sql_select = "SELECT hands_count FROM UnplausibleHands WHERE hand = \"" + hand.str() + "\"";
        sql_select += " AND betting_round = \"" + std::to_string(bettingRound) + "\"";
        sql_select += " AND nb_players = " + std::to_string(nbPlayers);
        sql_select += " AND human_player = " + std::to_string(human) + ";";
        if (sqlite3_get_table(mySqliteLogDb, sql_select.c_str(), &result, &nRow, &nCol, &errmsg) != SQLITE_OK)
        {
            GlobalServices::instance().logger()->error("Error in statement: " + sql_select + "[" + errmsg + "].");
        }
        else
        {
            if (nRow == 1)
            {
                previousCount = atoi(result[1]);
            }
        }
        sqlite3_free_table(result);
    }

    sql += "UPDATE UnplausibleHands SET hands_count = " + std::to_string(++previousCount);
    sql += " WHERE hand = '" + hand.str() + "'";
    sql += " AND betting_round = '" + std::to_string(bettingRound) + "'";
    sql += " AND nb_players = " + std::to_string(nbPlayers);
    sql += " AND human_player = " + std::to_string(human) + ";";

    exec_transaction();
}
void SqliteLogStore::createUnplausibleHandsTable()
{

    // create table if doesn't exist
    sql = "CREATE TABLE IF NOT EXISTS UnplausibleHands (";
    sql += "hand CHAR(3) NOT NULL";
    sql += ",betting_round CHAR";
    sql += ",nb_players INT";
    sql += ",human_player CHAR";
    sql += ",hands_count LARGEINT DEFAULT 0";
    sql += ", PRIMARY KEY(hand, betting_round, nb_players, human_player));";
    exec_transaction();
}

void SqliteLogStore::exec_transaction()
{
    char* errmsg = NULL;

    string sql_transaction = "BEGIN;" + sql + "COMMIT;";
    sql = "";
    // cout << endl << "SQL : " << sql_transaction << endl << endl;
    if (sqlite3_exec(mySqliteLogDb, sql_transaction.c_str(), 0, 0, &errmsg) != SQLITE_OK)
    {
        GlobalServices::instance().logger()->error("Error in statement: " + sql_transaction + "[" + errmsg + "].");
        sqlite3_free(errmsg);
        errmsg = NULL;
    }
}

void SqliteLogStore::updatePlayersStatistics(PlayerList activePlayerList)
{

    PlayerListConstIterator it_c;

    const int i = activePlayerList->size();

    for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
    {

        if ((*it_c)->getStatistics(i).getPreflopStatistics().m_hands == 0)
        {
            return;
        }

        sql = "UPDATE PlayersStatistics SET ";

        sql += "pf_hands = " + std::to_string((*it_c)->getStatistics(i).getPreflopStatistics().m_hands);
        sql += ",pf_checks = " + std::to_string((*it_c)->getStatistics(i).getPreflopStatistics().m_checks);
        sql += ",pf_calls = " + std::to_string((*it_c)->getStatistics(i).getPreflopStatistics().m_calls);
        sql += ",pf_raises = " + std::to_string((*it_c)->getStatistics(i).getPreflopStatistics().m_raises);
        sql += ",pf_3Bets = " + std::to_string((*it_c)->getStatistics(i).getPreflopStatistics().m_3Bets);
        sql += ",pf_call3Bets = " + std::to_string((*it_c)->getStatistics(i).getPreflopStatistics().m_call3Bets);
        sql += ",pf_call3BetsOpportunities = " +
               std::to_string((*it_c)->getStatistics(i).getPreflopStatistics().m_call3BetsOpportunities);
        sql += ",pf_4Bets = " + std::to_string((*it_c)->getStatistics(i).getPreflopStatistics().m_4Bets);
        sql += ",pf_folds = " + std::to_string((*it_c)->getStatistics(i).getPreflopStatistics().m_folds);
        sql += ",pf_limps = " + std::to_string((*it_c)->getStatistics(i).getPreflopStatistics().m_limps);

        sql += ",f_hands = " + std::to_string((*it_c)->getStatistics(i).getFlopStatistics().m_hands);
        sql += ",f_bets = " + std::to_string((*it_c)->getStatistics(i).getFlopStatistics().m_bets);
        sql += ",f_checks = " + std::to_string((*it_c)->getStatistics(i).getFlopStatistics().m_checks);
        sql += ",f_calls = " + std::to_string((*it_c)->getStatistics(i).getFlopStatistics().m_calls);
        sql += ",f_raises = " + std::to_string((*it_c)->getStatistics(i).getFlopStatistics().m_raises);
        sql += ",f_3Bets = " + std::to_string((*it_c)->getStatistics(i).getFlopStatistics().m_3Bets);
        sql += ",f_4Bets = " + std::to_string((*it_c)->getStatistics(i).getFlopStatistics().m_4Bets);
        sql += ",f_folds = " + std::to_string((*it_c)->getStatistics(i).getFlopStatistics().m_folds);
        sql +=
            ",f_continuationBets = " + std::to_string((*it_c)->getStatistics(i).getFlopStatistics().m_continuationBets);
        sql += ",f_continuationBetsOpportunities = " +
               std::to_string((*it_c)->getStatistics(i).getFlopStatistics().m_continuationBetsOpportunities);

        sql += ",t_hands = " + std::to_string((*it_c)->getStatistics(i).getTurnStatistics().m_hands);
        sql += ",t_checks = " + std::to_string((*it_c)->getStatistics(i).getTurnStatistics().m_checks);
        sql += ",t_bets = " + std::to_string((*it_c)->getStatistics(i).getTurnStatistics().m_bets);
        sql += ",t_calls = " + std::to_string((*it_c)->getStatistics(i).getTurnStatistics().m_calls);
        sql += ",t_raises = " + std::to_string((*it_c)->getStatistics(i).getTurnStatistics().m_raises);
        sql += ",t_3Bets = " + std::to_string((*it_c)->getStatistics(i).getTurnStatistics().m_3Bets);
        sql += ",t_4Bets = " + std::to_string((*it_c)->getStatistics(i).getTurnStatistics().m_4Bets);
        sql += ",t_folds = " + std::to_string((*it_c)->getStatistics(i).getTurnStatistics().m_folds);

        sql += ",r_hands = " + std::to_string((*it_c)->getStatistics(i).getRiverStatistics().m_hands);
        sql += ",r_checks = " + std::to_string((*it_c)->getStatistics(i).getRiverStatistics().m_checks);
        sql += ",r_bets = " + std::to_string((*it_c)->getStatistics(i).getRiverStatistics().m_bets);
        sql += ",r_calls = " + std::to_string((*it_c)->getStatistics(i).getRiverStatistics().m_calls);
        sql += ",r_raises = " + std::to_string((*it_c)->getStatistics(i).getRiverStatistics().m_raises);
        sql += ",r_3Bets = " + std::to_string((*it_c)->getStatistics(i).getRiverStatistics().m_3Bets);
        sql += ",r_4Bets = " + std::to_string((*it_c)->getStatistics(i).getRiverStatistics().m_4Bets);
        sql += ",r_folds = " + std::to_string((*it_c)->getStatistics(i).getRiverStatistics().m_folds);

        sql +=
            " WHERE strategy_name = '" + (*it_c)->getStrategyName() + "' AND nb_players = " + std::to_string(i) + ";";

        exec_transaction();
    }
}

std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1>
SqliteLogStore::getPlayerStatistics(const std::string& playerName)
{
    std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1> playerStatistics{};

    sqlite3* db;
    std::filesystem::path sqliteLogFileName;

    if (!validateLogDirectory(sqliteLogFileName))
    {
        return playerStatistics;
    }

    if (!openDatabase(sqliteLogFileName, db))
    {
        return playerStatistics;
    }

    std::string sql_select = "SELECT * FROM PlayersStatistics WHERE strategy_name = '" + playerName + "';";
    sqlite3_stmt* stmt;

    if (prepareStatement(db, sql_select, stmt))
    {
        processQueryResults(stmt, playerStatistics);
        sqlite3_finalize(stmt);
    }

    sqlite3_close(db);
    return playerStatistics;
}
bool SqliteLogStore::validateLogDirectory(std::filesystem::path& sqliteLogFileName) const
{
    bool dirExists = std::filesystem::is_directory(myLogDir);

    if (myLogDir.empty() || !dirExists)
    {
        return false;
    }

    sqliteLogFileName.clear();
    sqliteLogFileName /= myLogDir;
    sqliteLogFileName /= std::string(SQL_LOG_FILE);

    return true;
}
bool SqliteLogStore::openDatabase(const std::filesystem::path& sqliteLogFileName, sqlite3*& db) const
{
    int rc = sqlite3_open(sqliteLogFileName.string().c_str(), &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }

    return true;
}
bool SqliteLogStore::prepareStatement(sqlite3* db, const std::string& sql_select, sqlite3_stmt*& stmt) const
{
    int rc = sqlite3_prepare_v2(db, sql_select.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return false;
    }

    return true;
}
void SqliteLogStore::processQueryResults(
    sqlite3_stmt* stmt, std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1>& playerStatistics) const
{
    int nCols = sqlite3_column_count(stmt);

    if (nCols == 0)
    {
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int nbPlayers = 0;

        for (int nCol = 0; nCol < nCols; nCol++)
        {
            std::string columnName = sqlite3_column_name(stmt, nCol);

            if (columnName == "nb_players")
            {
                nbPlayers = sqlite3_column_int(stmt, nCol);
            }
            else
            {
                updatePlayerStatistics(playerStatistics[nbPlayers], columnName, stmt, nCol);
            }
        }
    }
}
void SqliteLogStore::updatePlayerStatistics(PlayerStatistics& stats, const std::string& columnName, sqlite3_stmt* stmt,
                                            int nCol) const
{
    static const std::unordered_map<std::string, std::function<void(PlayerStatistics&, int)>> columnMapping = {
        // Preflop statistics
        {"pf_hands", [](PlayerStatistics& s, int v) { s.m_preflopStatistics.m_hands = v; }},
        {"pf_folds", [](PlayerStatistics& s, int v) { s.m_preflopStatistics.m_folds = v; }},
        {"pf_checks", [](PlayerStatistics& s, int v) { s.m_preflopStatistics.m_checks = v; }},
        {"pf_calls", [](PlayerStatistics& s, int v) { s.m_preflopStatistics.m_calls = v; }},
        {"pf_raises", [](PlayerStatistics& s, int v) { s.m_preflopStatistics.m_raises = v; }},
        {"pf_limps", [](PlayerStatistics& s, int v) { s.m_preflopStatistics.m_limps = v; }},
        {"pf_3Bets", [](PlayerStatistics& s, int v) { s.m_preflopStatistics.m_3Bets = v; }},
        {"pf_call3Bets", [](PlayerStatistics& s, int v) { s.m_preflopStatistics.m_call3Bets = v; }},
        {"pf_call3BetsOpportunities",
         [](PlayerStatistics& s, int v) { s.m_preflopStatistics.m_call3BetsOpportunities = v; }},
        {"pf_4Bets", [](PlayerStatistics& s, int v) { s.m_preflopStatistics.m_4Bets = v; }},
        // Flop statistics
        {"f_hands", [](PlayerStatistics& s, int v) { s.m_flopStatistics.m_hands = v; }},
        {"f_folds", [](PlayerStatistics& s, int v) { s.m_flopStatistics.m_folds = v; }},
        {"f_checks", [](PlayerStatistics& s, int v) { s.m_flopStatistics.m_checks = v; }},
        {"f_calls", [](PlayerStatistics& s, int v) { s.m_flopStatistics.m_calls = v; }},
        {"f_raises", [](PlayerStatistics& s, int v) { s.m_flopStatistics.m_raises = v; }},
        {"f_bets", [](PlayerStatistics& s, int v) { s.m_flopStatistics.m_bets = v; }},
        {"f_3Bets", [](PlayerStatistics& s, int v) { s.m_flopStatistics.m_3Bets = v; }},
        {"f_4Bets", [](PlayerStatistics& s, int v) { s.m_flopStatistics.m_4Bets = v; }},
        {"f_continuationBets", [](PlayerStatistics& s, int v) { s.m_flopStatistics.m_continuationBets = v; }},
        {"f_continuationBetsOpportunities",
         [](PlayerStatistics& s, int v) { s.m_flopStatistics.m_continuationBetsOpportunities = v; }},
        // Turn statistics
        {"t_hands", [](PlayerStatistics& s, int v) { s.m_turnStatistics.m_hands = v; }},
        {"t_folds", [](PlayerStatistics& s, int v) { s.m_turnStatistics.m_folds = v; }},
        {"t_checks", [](PlayerStatistics& s, int v) { s.m_turnStatistics.m_checks = v; }},
        {"t_calls", [](PlayerStatistics& s, int v) { s.m_turnStatistics.m_calls = v; }},
        {"t_raises", [](PlayerStatistics& s, int v) { s.m_turnStatistics.m_raises = v; }},
        {"t_bets", [](PlayerStatistics& s, int v) { s.m_turnStatistics.m_bets = v; }},
        {"t_3Bets", [](PlayerStatistics& s, int v) { s.m_turnStatistics.m_3Bets = v; }},
        {"t_4Bets", [](PlayerStatistics& s, int v) { s.m_turnStatistics.m_4Bets = v; }},
        // River statistics
        {"r_hands", [](PlayerStatistics& s, int v) { s.m_riverStatistics.m_hands = v; }},
        {"r_folds", [](PlayerStatistics& s, int v) { s.m_riverStatistics.m_folds = v; }},
        {"r_checks", [](PlayerStatistics& s, int v) { s.m_riverStatistics.m_checks = v; }},
        {"r_calls", [](PlayerStatistics& s, int v) { s.m_riverStatistics.m_calls = v; }},
        {"r_raises", [](PlayerStatistics& s, int v) { s.m_riverStatistics.m_raises = v; }},
        {"r_bets", [](PlayerStatistics& s, int v) { s.m_riverStatistics.m_bets = v; }},
        {"r_3Bets", [](PlayerStatistics& s, int v) { s.m_riverStatistics.m_3Bets = v; }},
        {"r_4Bets", [](PlayerStatistics& s, int v) { s.m_riverStatistics.m_4Bets = v; }},
    };

    auto it = columnMapping.find(columnName);
    if (it != columnMapping.end())
    {
        // we call the lambda function associated with the column name, passing the stats and the value
        // from the sqlite statement
        it->second(stats, sqlite3_column_int(stmt, nCol));
    }
}
} // namespace pkt::infra
