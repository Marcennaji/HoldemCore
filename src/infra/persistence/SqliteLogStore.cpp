// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "SqliteLogStore.h"
#include <core/player/typedefs.h>
#include "core/engine/CardUtilities.h"
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

SqliteLogStore::SqliteLogStore(const std::string& logDir) : mySqliteLogFileName(""), myLogDir(logDir), mySql("")
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

        if (mySqliteLogDb != nullptr)
        {

            if (!databaseExists)
            {
                createDatabase();
            }
        }
    }
}

void SqliteLogStore::createDatabase()
{

    createRankingTable();
    createUnplausibleHandsTable();

    // create stats table
    mySql += "CREATE TABLE  IF NOT EXISTS PlayersStatistics (";
    mySql += "strategy_name VARCHAR NOT NULL";
    mySql += ", nb_players SMALLINT NOT NULL";
    mySql += ", pf_hands LARGEINT ";
    // preflop stats :
    mySql += ", pf_folds LARGEINT ";
    mySql += ", pf_limps LARGEINT ";
    mySql += ", pf_checks LARGEINT ";
    mySql += ", pf_calls LARGEINT ";
    mySql += ", pf_raises LARGEINT ";
    mySql += ", pf_3Bets LARGEINT ";
    mySql += ", pf_call3Bets LARGEINT ";
    mySql += ", pf_call3BetsOpportunities LARGEINT ";
    mySql += ", pf_4Bets LARGEINT ";
    // flop stats :
    mySql += ", f_hands LARGEINT ";
    mySql += ", f_folds LARGEINT ";
    mySql += ", f_checks LARGEINT ";
    mySql += ", f_bets LARGEINT ";
    mySql += ", f_calls LARGEINT ";
    mySql += ", f_raises LARGEINT ";
    mySql += ", f_3Bets LARGEINT ";
    mySql += ", f_4Bets LARGEINT ";
    mySql += ", f_continuationBets LARGEINT ";
    mySql += ", f_continuationBetsOpportunities LARGEINT ";
    // turn stats :
    mySql += ", t_hands LARGEINT ";
    mySql += ", t_folds LARGEINT ";
    mySql += ", t_checks LARGEINT ";
    mySql += ", t_calls LARGEINT ";
    mySql += ", t_bets LARGEINT ";
    mySql += ", t_raises LARGEINT ";
    mySql += ", t_3Bets LARGEINT ";
    mySql += ", t_4Bets LARGEINT ";
    // river stats :
    mySql += ", r_hands LARGEINT ";
    mySql += ", r_folds LARGEINT ";
    mySql += ", r_bets LARGEINT ";
    mySql += ", r_checks LARGEINT ";
    mySql += ", r_calls LARGEINT ";
    mySql += ", r_raises LARGEINT ";
    mySql += ", r_3Bets LARGEINT ";
    mySql += ", r_4Bets LARGEINT ";

    mySql += ", PRIMARY KEY(strategy_name, nb_players));";

    execTransaction();

    auto looseAggressiveStrategy = LooseAggressiveBotStrategy();
    auto tightAggressiveStrategy = TightAggressiveBotStrategy();
    auto maniacStrategy = ManiacBotStrategy();
    auto ultraTightStrategy = UltraTightBotStrategy();

    for (int j = 2; j <= MAX_NUMBER_OF_PLAYERS; j++)
    {
        initializeStrategyStatistics("You", j); // human player
        // initialize players statistics for all bot strategies
        initializeStrategyStatistics(tightAggressiveStrategy.getStrategyName(), j);
        initializeStrategyStatistics(looseAggressiveStrategy.getStrategyName(), j);
        initializeStrategyStatistics(maniacStrategy.getStrategyName(), j);
        initializeStrategyStatistics(ultraTightStrategy.getStrategyName(), j);
    }
}

void SqliteLogStore::initializeStrategyStatistics(const string playerName, const int nbPlayers)
{

    mySql += "INSERT OR REPLACE INTO PlayersStatistics (";
    mySql += "strategy_name,nb_players";
    mySql += ",pf_hands,pf_checks,pf_calls,pf_raises,pf_3Bets,pf_call3Bets,pf_call3BetsOpportunities,pf_4Bets,pf_folds,"
             "pf_limps";
    mySql += ",f_hands,f_checks,f_bets,f_calls,f_raises,f_3Bets,f_4Bets,f_folds,f_continuationBets,f_"
             "continuationBetsOpportunities";
    mySql += ",t_hands,t_checks,t_bets,t_calls,t_raises,t_3Bets,t_4Bets,t_folds";
    mySql += ",r_hands,r_checks,r_bets,r_calls,r_raises,r_3Bets,r_4Bets,r_folds";
    mySql += ") VALUES (";
    mySql += "'";
    mySql += playerName;
    mySql += "',";
    mySql += std::to_string(nbPlayers);
    mySql += ",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);";

    execTransaction();
}

void SqliteLogStore::updateRankingGameLosers(PlayerList activePlayerList)
{
    createRankingTable();

    int losers = 0;
    PlayerListConstIterator itC;
    for (itC = activePlayerList->begin(); itC != activePlayerList->end(); ++itC)
    {
        if ((*itC)->getCash() == 0)
        {
            mySql += "INSERT OR IGNORE INTO Ranking VALUES ('" + (*itC)->getStrategyName() + "', 0, 0, 0);";
            const int lostStack = getIntegerValue((*itC)->getStrategyName(), "Ranking", "lost_stack") + 1;
            mySql += "UPDATE Ranking SET lost_stack = " + std::to_string(lostStack);
            mySql += " WHERE strategy_name = '" + (*itC)->getStrategyName() + "';";
        }
    }

    execTransaction();
}
void SqliteLogStore::updateRankingGameWinner(PlayerList activePlayerList)
{
    createRankingTable();

    int playersPositiveCashCounter = 0;
    PlayerListConstIterator itC;
    for (itC = activePlayerList->begin(); itC != activePlayerList->end(); ++itC)
    {
        if ((*itC)->getCash() > 0)
        {
            playersPositiveCashCounter++;
        }
    }
    if (playersPositiveCashCounter == 1)
    {
        for (itC = activePlayerList->begin(); itC != activePlayerList->end(); ++itC)
        {
            if ((*itC)->getCash() > 0)
            {
                mySql += "INSERT OR IGNORE INTO Ranking VALUES ('" + (*itC)->getStrategyName() + "', 0, 0, 0);";
                const int wonGame = getIntegerValue((*itC)->getStrategyName(), "Ranking", "won_game") + 1;
                mySql += "UPDATE Ranking SET won_game = " + std::to_string(wonGame);
                mySql += " WHERE strategy_name = '" + (*itC)->getStrategyName() + "';";
            }
        }
    }

    execTransaction();
}
void SqliteLogStore::updateRankingPlayedGames(PlayerList activePlayerList)
{
    createRankingTable();

    int losers = 0;
    PlayerListConstIterator itC;
    for (itC = activePlayerList->begin(); itC != activePlayerList->end(); ++itC)
    {
        mySql += "INSERT OR IGNORE INTO Ranking VALUES ('" + (*itC)->getStrategyName() + "', 0, 0, 0);";
        const int playedGames = getIntegerValue((*itC)->getStrategyName(), "Ranking", "played_games") + 1;
        mySql += "UPDATE Ranking SET played_games = " + std::to_string(playedGames);
        mySql += " WHERE strategy_name = '" + (*itC)->getStrategyName() + "';";
    }

    execTransaction();
}
int SqliteLogStore::getIntegerValue(const std::string playerName, const std::string tableName,
                                    const std::string attributeName)
{

    int result = 0;

    if (mySqliteLogDb != nullptr)
    {
        // sqlite-db is open

        char** resultPlayer = nullptr;
        int nRowPlayer = 0;
        int nColPlayer = 0;
        char* errmsg = nullptr;

        // read seat
        string sqlSelect =
            "SELECT " + attributeName + " FROM " + tableName + " WHERE strategy_name = \"" + playerName + "\"";
        if (sqlite3_get_table(mySqliteLogDb, sqlSelect.c_str(), &resultPlayer, &nRowPlayer, &nColPlayer, &errmsg) !=
            SQLITE_OK)
        {
            GlobalServices::instance().logger()->error("Error in statement: " + sqlSelect + "[" + errmsg + "].");
        }
        else
        {
            if (nRowPlayer == 1)
            {
                result = atoi(resultPlayer[1]);
            }
            else
            {
                GlobalServices::instance().logger()->info("no data for player " + playerName + " in table " +
                                                          tableName);
                GlobalServices::instance().logger()->info("SQL query was :\n" + sqlSelect);
            }
        }
        sqlite3_free_table(resultPlayer);
    }
    return result;
}

void SqliteLogStore::createRankingTable()
{

    // create table if doesn't exist
    mySql = "CREATE TABLE IF NOT EXISTS Ranking (";
    mySql += "strategy_name VARCHAR NOT NULL";
    mySql += ",lost_stack LARGEINT";
    mySql += ",won_game LARGEINT";
    mySql += ",played_games LARGEINT";
    mySql += ", PRIMARY KEY(strategy_name));";
    execTransaction();
}
void SqliteLogStore::updateUnplausibleHand(const std::string card1, const std::string card2, const bool human,
                                           const char bettingRound, const int nbPlayers)
{
    createUnplausibleHandsTable();

    std::stringstream hand;

    if (card1.at(1) == card2.at(1))
    {
        // suited hand
        if (CardUtilities::getCardValue(card1) > CardUtilities::getCardValue(card2))
        {
            hand << card1 << card2;
        }
        else if (CardUtilities::getCardValue(card1) < CardUtilities::getCardValue(card2))
        {
            hand << card2 << card1;
        }
    }
    else
    {
        // unsuited hand
        if (CardUtilities::getCardValue(card1) > CardUtilities::getCardValue(card2))
        {
            hand << card1.at(0) << card2.at(0) << 'o';
        }
        else if (CardUtilities::getCardValue(card1) < CardUtilities::getCardValue(card2))
        {
            hand << card2.at(0) << card1.at(0) << 'o';
        }
        else
        {
            hand << card1.at(0) << card2.at(0);
        }
    }

    int losers = 0;
    mySql += "INSERT OR IGNORE INTO UnplausibleHands VALUES ('" + hand.str() + "','" + bettingRound + "'," +
             std::to_string(nbPlayers) + (human ? ", 1" : ", 0") + ", 0);";

    // get previous count value
    int previousCount = 0;

    if (mySqliteLogDb != nullptr)
    {

        char** result = nullptr;
        int nRow = 0;
        int nCol = 0;
        char* errmsg = nullptr;

        // read seat
        string sqlSelect = "SELECT hands_count FROM UnplausibleHands WHERE hand = \"" + hand.str() + "\"";
        sqlSelect += " AND betting_round = \"" + std::to_string(bettingRound) + "\"";
        sqlSelect += " AND nb_players = " + std::to_string(nbPlayers);
        sqlSelect += " AND human_player = " + std::to_string(human) + ";";
        if (sqlite3_get_table(mySqliteLogDb, sqlSelect.c_str(), &result, &nRow, &nCol, &errmsg) != SQLITE_OK)
        {
            GlobalServices::instance().logger()->error("Error in statement: " + sqlSelect + "[" + errmsg + "].");
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

    mySql += "UPDATE UnplausibleHands SET hands_count = " + std::to_string(++previousCount);
    mySql += " WHERE hand = '" + hand.str() + "'";
    mySql += " AND betting_round = '" + std::to_string(bettingRound) + "'";
    mySql += " AND nb_players = " + std::to_string(nbPlayers);
    mySql += " AND human_player = " + std::to_string(human) + ";";

    execTransaction();
}
void SqliteLogStore::createUnplausibleHandsTable()
{

    // create table if doesn't exist
    mySql = "CREATE TABLE IF NOT EXISTS UnplausibleHands (";
    mySql += "hand CHAR(3) NOT NULL";
    mySql += ",betting_round CHAR";
    mySql += ",nb_players INT";
    mySql += ",human_player CHAR";
    mySql += ",hands_count LARGEINT DEFAULT 0";
    mySql += ", PRIMARY KEY(hand, betting_round, nb_players, human_player));";
    execTransaction();
}

void SqliteLogStore::execTransaction()
{
    char* errmsg = nullptr;

    string sqlTransaction = "BEGIN;" + mySql + "COMMIT;";
    mySql = "";
    // cout << endl << "SQL : " << sql_transaction << endl << endl;
    if (sqlite3_exec(mySqliteLogDb, sqlTransaction.c_str(), nullptr, nullptr, &errmsg) != SQLITE_OK)
    {
        GlobalServices::instance().logger()->error("Error in statement: " + sqlTransaction + "[" + errmsg + "].");
        sqlite3_free(errmsg);
        errmsg = nullptr;
    }
}

void SqliteLogStore::updatePlayersStatistics(PlayerList activePlayerList)
{

    PlayerListConstIterator itC;

    const int i = activePlayerList->size();

    for (itC = activePlayerList->begin(); itC != activePlayerList->end(); ++itC)
    {

        if ((*itC)->getStatistics(i).getPreflopStatistics().m_hands == 0)
        {
            return;
        }

        mySql = "UPDATE PlayersStatistics SET ";

        mySql += "pf_hands = " + std::to_string((*itC)->getStatistics(i).getPreflopStatistics().m_hands);
        mySql += ",pf_checks = " + std::to_string((*itC)->getStatistics(i).getPreflopStatistics().m_checks);
        mySql += ",pf_calls = " + std::to_string((*itC)->getStatistics(i).getPreflopStatistics().m_calls);
        mySql += ",pf_raises = " + std::to_string((*itC)->getStatistics(i).getPreflopStatistics().m_raises);
        mySql += ",pf_3Bets = " + std::to_string((*itC)->getStatistics(i).getPreflopStatistics().m_3Bets);
        mySql += ",pf_call3Bets = " + std::to_string((*itC)->getStatistics(i).getPreflopStatistics().m_call3Bets);
        mySql += ",pf_call3BetsOpportunities = " +
                 std::to_string((*itC)->getStatistics(i).getPreflopStatistics().m_call3BetsOpportunities);
        mySql += ",pf_4Bets = " + std::to_string((*itC)->getStatistics(i).getPreflopStatistics().m_4Bets);
        mySql += ",pf_folds = " + std::to_string((*itC)->getStatistics(i).getPreflopStatistics().m_folds);
        mySql += ",pf_limps = " + std::to_string((*itC)->getStatistics(i).getPreflopStatistics().m_limps);

        mySql += ",f_hands = " + std::to_string((*itC)->getStatistics(i).getFlopStatistics().m_hands);
        mySql += ",f_bets = " + std::to_string((*itC)->getStatistics(i).getFlopStatistics().m_bets);
        mySql += ",f_checks = " + std::to_string((*itC)->getStatistics(i).getFlopStatistics().m_checks);
        mySql += ",f_calls = " + std::to_string((*itC)->getStatistics(i).getFlopStatistics().m_calls);
        mySql += ",f_raises = " + std::to_string((*itC)->getStatistics(i).getFlopStatistics().m_raises);
        mySql += ",f_3Bets = " + std::to_string((*itC)->getStatistics(i).getFlopStatistics().m_3Bets);
        mySql += ",f_4Bets = " + std::to_string((*itC)->getStatistics(i).getFlopStatistics().m_4Bets);
        mySql += ",f_folds = " + std::to_string((*itC)->getStatistics(i).getFlopStatistics().m_folds);
        mySql +=
            ",f_continuationBets = " + std::to_string((*itC)->getStatistics(i).getFlopStatistics().m_continuationBets);
        mySql += ",f_continuationBetsOpportunities = " +
                 std::to_string((*itC)->getStatistics(i).getFlopStatistics().m_continuationBetsOpportunities);

        mySql += ",t_hands = " + std::to_string((*itC)->getStatistics(i).getTurnStatistics().m_hands);
        mySql += ",t_checks = " + std::to_string((*itC)->getStatistics(i).getTurnStatistics().m_checks);
        mySql += ",t_bets = " + std::to_string((*itC)->getStatistics(i).getTurnStatistics().m_bets);
        mySql += ",t_calls = " + std::to_string((*itC)->getStatistics(i).getTurnStatistics().m_calls);
        mySql += ",t_raises = " + std::to_string((*itC)->getStatistics(i).getTurnStatistics().m_raises);
        mySql += ",t_3Bets = " + std::to_string((*itC)->getStatistics(i).getTurnStatistics().m_3Bets);
        mySql += ",t_4Bets = " + std::to_string((*itC)->getStatistics(i).getTurnStatistics().m_4Bets);
        mySql += ",t_folds = " + std::to_string((*itC)->getStatistics(i).getTurnStatistics().m_folds);

        mySql += ",r_hands = " + std::to_string((*itC)->getStatistics(i).getRiverStatistics().m_hands);
        mySql += ",r_checks = " + std::to_string((*itC)->getStatistics(i).getRiverStatistics().m_checks);
        mySql += ",r_bets = " + std::to_string((*itC)->getStatistics(i).getRiverStatistics().m_bets);
        mySql += ",r_calls = " + std::to_string((*itC)->getStatistics(i).getRiverStatistics().m_calls);
        mySql += ",r_raises = " + std::to_string((*itC)->getStatistics(i).getRiverStatistics().m_raises);
        mySql += ",r_3Bets = " + std::to_string((*itC)->getStatistics(i).getRiverStatistics().m_3Bets);
        mySql += ",r_4Bets = " + std::to_string((*itC)->getStatistics(i).getRiverStatistics().m_4Bets);
        mySql += ",r_folds = " + std::to_string((*itC)->getStatistics(i).getRiverStatistics().m_folds);

        mySql +=
            " WHERE strategy_name = '" + (*itC)->getStrategyName() + "' AND nb_players = " + std::to_string(i) + ";";

        execTransaction();
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

    std::string sqlSelect = "SELECT * FROM PlayersStatistics WHERE strategy_name = '" + playerName + "';";
    sqlite3_stmt* stmt;

    if (prepareStatement(db, sqlSelect, stmt))
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
bool SqliteLogStore::prepareStatement(sqlite3* db, const std::string& sqlSelect, sqlite3_stmt*& stmt) const
{
    int rc = sqlite3_prepare_v2(db, sqlSelect.c_str(), -1, &stmt, nullptr);

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
