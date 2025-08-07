// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "SqliteLogStore.h"
#include <core/player/typedefs.h>
#include "core/engine/CardUtilities.h"
#include "core/engine/EngineDefs.h"
#include "core/interfaces/NullLogger.h"
#include "core/player/HumanPlayer.h"
#include "core/player/Player.h"
#include "core/player/PlayerStatistics.h"
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

SqliteLogStore::SqliteLogStore(const std::string& logDir) : mySqliteLogFileName(""), mySql(""), myLogDir(logDir)
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

            GlobalServices::instance().logger()->verbose("warning : database does not exist, will be created");
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
    mySql += ", pf_threeBets LARGEINT ";
    mySql += ", pf_callthreeBets LARGEINT ";
    mySql += ", pf_callthreeBetsOpportunities LARGEINT ";
    mySql += ", pf_fourBets LARGEINT ";
    // flop stats :
    mySql += ", f_hands LARGEINT ";
    mySql += ", f_folds LARGEINT ";
    mySql += ", f_checks LARGEINT ";
    mySql += ", f_bets LARGEINT ";
    mySql += ", f_calls LARGEINT ";
    mySql += ", f_raises LARGEINT ";
    mySql += ", f_threeBets LARGEINT ";
    mySql += ", f_fourBets LARGEINT ";
    mySql += ", f_continuationBets LARGEINT ";
    mySql += ", f_continuationBetsOpportunities LARGEINT ";
    // turn stats :
    mySql += ", t_hands LARGEINT ";
    mySql += ", t_folds LARGEINT ";
    mySql += ", t_checks LARGEINT ";
    mySql += ", t_calls LARGEINT ";
    mySql += ", t_bets LARGEINT ";
    mySql += ", t_raises LARGEINT ";
    mySql += ", t_threeBets LARGEINT ";
    mySql += ", t_fourBets LARGEINT ";
    // river stats :
    mySql += ", r_hands LARGEINT ";
    mySql += ", r_folds LARGEINT ";
    mySql += ", r_bets LARGEINT ";
    mySql += ", r_checks LARGEINT ";
    mySql += ", r_calls LARGEINT ";
    mySql += ", r_raises LARGEINT ";
    mySql += ", r_threeBets LARGEINT ";
    mySql += ", r_fourBets LARGEINT ";

    mySql += ", PRIMARY KEY(strategy_name, nb_players));";

    execTransaction();

    auto looseAggressiveStrategy = LooseAggressiveBotStrategy();
    auto tightAggressiveStrategy = TightAggressiveBotStrategy();
    auto maniacStrategy = ManiacBotStrategy();
    auto ultraTightStrategy = UltraTightBotStrategy();

    for (int j = 2; j <= MAX_NUMBER_OF_PLAYERS; j++)
    {
        initializeStrategyStatistics(HumanPlayer::getName(), j);
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
    mySql += ",pf_hands,pf_checks,pf_calls,pf_raises,pf_threeBets,pf_callthreeBets,pf_callthreeBetsOpportunities,pf_"
             "fourBets,pf_folds,"
             "pf_limps";
    mySql += ",f_hands,f_checks,f_bets,f_calls,f_raises,f_threeBets,f_fourBets,f_folds,f_continuationBets,f_"
             "continuationBetsOpportunities";
    mySql += ",t_hands,t_checks,t_bets,t_calls,t_raises,t_threeBets,t_fourBets,t_folds";
    mySql += ",r_hands,r_checks,r_bets,r_calls,r_raises,r_threeBets,r_fourBets,r_folds";
    mySql += ") VALUES (";
    mySql += "'";
    mySql += playerName;
    mySql += "',";
    mySql += std::to_string(nbPlayers);
    mySql += ",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);";

    execTransaction();
}

void SqliteLogStore::updateRankingGameLosers(PlayerList seatsList)
{
    createRankingTable();

    int losers = 0;
    PlayerListConstIterator itC;
    for (itC = seatsList->begin(); itC != seatsList->end(); ++itC)
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
void SqliteLogStore::updateRankingGameWinner(PlayerList seatsList)
{
    createRankingTable();

    int playersPositiveCashCounter = 0;
    PlayerListConstIterator itC;
    for (itC = seatsList->begin(); itC != seatsList->end(); ++itC)
    {
        if ((*itC)->getCash() > 0)
        {
            playersPositiveCashCounter++;
        }
    }
    if (playersPositiveCashCounter == 1)
    {
        for (itC = seatsList->begin(); itC != seatsList->end(); ++itC)
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
void SqliteLogStore::updateRankingPlayedGames(PlayerList seatsList)
{
    createRankingTable();

    int losers = 0;
    PlayerListConstIterator itC;
    for (itC = seatsList->begin(); itC != seatsList->end(); ++itC)
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
                GlobalServices::instance().logger()->verbose("no data for player " + playerName + " in table " +
                                                             tableName);
                GlobalServices::instance().logger()->verbose("SQL query was :\n" + sqlSelect);
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

void SqliteLogStore::updatePlayersStatistics(PlayerList seatsList)
{

    PlayerListConstIterator itC;

    const int i = seatsList->size();

    for (itC = seatsList->begin(); itC != seatsList->end(); ++itC)
    {

        if ((*itC)->getStatistics(i).preflopStatistics.hands == 0)
        {
            return;
        }

        mySql = "UPDATE PlayersStatistics SET ";

        mySql += "pf_hands = " + std::to_string((*itC)->getStatistics(i).preflopStatistics.hands);
        mySql += ",pf_checks = " + std::to_string((*itC)->getStatistics(i).preflopStatistics.checks);
        mySql += ",pf_calls = " + std::to_string((*itC)->getStatistics(i).preflopStatistics.calls);
        mySql += ",pf_raises = " + std::to_string((*itC)->getStatistics(i).preflopStatistics.raises);
        mySql += ",pf_threeBets = " + std::to_string((*itC)->getStatistics(i).preflopStatistics.threeBets);
        mySql += ",pf_callthreeBets = " + std::to_string((*itC)->getStatistics(i).preflopStatistics.callthreeBets);
        mySql += ",pf_callthreeBetsOpportunities = " +
                 std::to_string((*itC)->getStatistics(i).preflopStatistics.callthreeBetsOpportunities);
        mySql += ",pf_fourBets = " + std::to_string((*itC)->getStatistics(i).preflopStatistics.fourBets);
        mySql += ",pf_folds = " + std::to_string((*itC)->getStatistics(i).preflopStatistics.folds);
        mySql += ",pf_limps = " + std::to_string((*itC)->getStatistics(i).preflopStatistics.limps);

        mySql += ",f_hands = " + std::to_string((*itC)->getStatistics(i).flopStatistics.hands);
        mySql += ",f_bets = " + std::to_string((*itC)->getStatistics(i).flopStatistics.bets);
        mySql += ",f_checks = " + std::to_string((*itC)->getStatistics(i).flopStatistics.checks);
        mySql += ",f_calls = " + std::to_string((*itC)->getStatistics(i).flopStatistics.calls);
        mySql += ",f_raises = " + std::to_string((*itC)->getStatistics(i).flopStatistics.raises);
        mySql += ",f_threeBets = " + std::to_string((*itC)->getStatistics(i).flopStatistics.threeBets);
        mySql += ",f_fourBets = " + std::to_string((*itC)->getStatistics(i).flopStatistics.fourBets);
        mySql += ",f_folds = " + std::to_string((*itC)->getStatistics(i).flopStatistics.folds);
        mySql += ",f_continuationBets = " + std::to_string((*itC)->getStatistics(i).flopStatistics.continuationBets);
        mySql += ",f_continuationBetsOpportunities = " +
                 std::to_string((*itC)->getStatistics(i).flopStatistics.continuationBetsOpportunities);

        mySql += ",t_hands = " + std::to_string((*itC)->getStatistics(i).turnStatistics.hands);
        mySql += ",t_checks = " + std::to_string((*itC)->getStatistics(i).turnStatistics.checks);
        mySql += ",t_bets = " + std::to_string((*itC)->getStatistics(i).turnStatistics.bets);
        mySql += ",t_calls = " + std::to_string((*itC)->getStatistics(i).turnStatistics.calls);
        mySql += ",t_raises = " + std::to_string((*itC)->getStatistics(i).turnStatistics.raises);
        mySql += ",t_threeBets = " + std::to_string((*itC)->getStatistics(i).turnStatistics.threeBets);
        mySql += ",t_fourBets = " + std::to_string((*itC)->getStatistics(i).turnStatistics.fourBets);
        mySql += ",t_folds = " + std::to_string((*itC)->getStatistics(i).turnStatistics.folds);

        mySql += ",r_hands = " + std::to_string((*itC)->getStatistics(i).riverStatistics.hands);
        mySql += ",r_checks = " + std::to_string((*itC)->getStatistics(i).riverStatistics.checks);
        mySql += ",r_bets = " + std::to_string((*itC)->getStatistics(i).riverStatistics.bets);
        mySql += ",r_calls = " + std::to_string((*itC)->getStatistics(i).riverStatistics.calls);
        mySql += ",r_raises = " + std::to_string((*itC)->getStatistics(i).riverStatistics.raises);
        mySql += ",r_threeBets = " + std::to_string((*itC)->getStatistics(i).riverStatistics.threeBets);
        mySql += ",r_fourBets = " + std::to_string((*itC)->getStatistics(i).riverStatistics.fourBets);
        mySql += ",r_folds = " + std::to_string((*itC)->getStatistics(i).riverStatistics.folds);

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
        {"pf_hands", [](PlayerStatistics& s, int v) { s.preflopStatistics.hands = v; }},
        {"pf_folds", [](PlayerStatistics& s, int v) { s.preflopStatistics.folds = v; }},
        {"pf_checks", [](PlayerStatistics& s, int v) { s.preflopStatistics.checks = v; }},
        {"pf_calls", [](PlayerStatistics& s, int v) { s.preflopStatistics.calls = v; }},
        {"pf_raises", [](PlayerStatistics& s, int v) { s.preflopStatistics.raises = v; }},
        {"pf_limps", [](PlayerStatistics& s, int v) { s.preflopStatistics.limps = v; }},
        {"pf_threeBets", [](PlayerStatistics& s, int v) { s.preflopStatistics.threeBets = v; }},
        {"pf_callthreeBets", [](PlayerStatistics& s, int v) { s.preflopStatistics.callthreeBets = v; }},
        {"pf_callthreeBetsOpportunities",
         [](PlayerStatistics& s, int v) { s.preflopStatistics.callthreeBetsOpportunities = v; }},
        {"pf_fourBets", [](PlayerStatistics& s, int v) { s.preflopStatistics.fourBets = v; }},
        // Flop statistics
        {"f_hands", [](PlayerStatistics& s, int v) { s.flopStatistics.hands = v; }},
        {"f_folds", [](PlayerStatistics& s, int v) { s.flopStatistics.folds = v; }},
        {"f_checks", [](PlayerStatistics& s, int v) { s.flopStatistics.checks = v; }},
        {"f_calls", [](PlayerStatistics& s, int v) { s.flopStatistics.calls = v; }},
        {"f_raises", [](PlayerStatistics& s, int v) { s.flopStatistics.raises = v; }},
        {"f_bets", [](PlayerStatistics& s, int v) { s.flopStatistics.bets = v; }},
        {"f_threeBets", [](PlayerStatistics& s, int v) { s.flopStatistics.threeBets = v; }},
        {"f_fourBets", [](PlayerStatistics& s, int v) { s.flopStatistics.fourBets = v; }},
        {"f_continuationBets", [](PlayerStatistics& s, int v) { s.flopStatistics.continuationBets = v; }},
        {"f_continuationBetsOpportunities",
         [](PlayerStatistics& s, int v) { s.flopStatistics.continuationBetsOpportunities = v; }},
        // Turn statistics
        {"t_hands", [](PlayerStatistics& s, int v) { s.turnStatistics.hands = v; }},
        {"t_folds", [](PlayerStatistics& s, int v) { s.turnStatistics.folds = v; }},
        {"t_checks", [](PlayerStatistics& s, int v) { s.turnStatistics.checks = v; }},
        {"t_calls", [](PlayerStatistics& s, int v) { s.turnStatistics.calls = v; }},
        {"t_raises", [](PlayerStatistics& s, int v) { s.turnStatistics.raises = v; }},
        {"t_bets", [](PlayerStatistics& s, int v) { s.turnStatistics.bets = v; }},
        {"t_threeBets", [](PlayerStatistics& s, int v) { s.turnStatistics.threeBets = v; }},
        {"t_fourBets", [](PlayerStatistics& s, int v) { s.turnStatistics.fourBets = v; }},
        // River statistics
        {"r_hands", [](PlayerStatistics& s, int v) { s.riverStatistics.hands = v; }},
        {"r_folds", [](PlayerStatistics& s, int v) { s.riverStatistics.folds = v; }},
        {"r_checks", [](PlayerStatistics& s, int v) { s.riverStatistics.checks = v; }},
        {"r_calls", [](PlayerStatistics& s, int v) { s.riverStatistics.calls = v; }},
        {"r_raises", [](PlayerStatistics& s, int v) { s.riverStatistics.raises = v; }},
        {"r_bets", [](PlayerStatistics& s, int v) { s.riverStatistics.bets = v; }},
        {"r_threeBets", [](PlayerStatistics& s, int v) { s.riverStatistics.threeBets = v; }},
        {"r_fourBets", [](PlayerStatistics& s, int v) { s.riverStatistics.fourBets = v; }},
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
