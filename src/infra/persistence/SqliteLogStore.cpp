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

#include "SqliteLogStore.h"
#include "core/engine/CardsValue.h"
#include "core/engine/EngineDefs.h"
#include "core/engine/model/PlayerStatistics.h"
#include "core/player/Player.h"

#include <third_party/sqlite3/sqlite3.h>

#include <array>
#include <sstream>
#include <sys/types.h>

namespace pkt::infra
{

using namespace std;
using namespace pkt::core;

SqliteLogStore::SqliteLogStore(const std::string& logDir)
    : mySqliteLogDb(0), mySqliteLogFileName(""), myLogDir(logDir), uniqueGameID(0), currentHandID(0), sql("")
{
}

SqliteLogStore::~SqliteLogStore()
{
    if (SQLITE_LOG)
    {
        sqlite3_close(mySqliteLogDb);
    }
}

void SqliteLogStore::init()
{

    if (SQLITE_LOG)
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
#ifdef LOG_POKER_EXEC
                cout << "warning : database does not exist, will be created" << endl;
#endif
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
}

void SqliteLogStore::createDatabase()
{

    createRankingTable();
    createUnplausibleHandsTable();

    // create stats table
    sql += "CREATE TABLE  IF NOT EXISTS PlayersStatistics (";
    sql += "player_name VARCHAR NOT NULL";
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

    sql += ", PRIMARY KEY(player_name, nb_players));";

    exec_transaction();

    for (int i = 0; i < MAX_NUMBER_OF_PLAYERS; i++)
    {

        for (int j = 2; j <= MAX_NUMBER_OF_PLAYERS; j++)
        {
            InitializePlayersStatistics(TightAggressiveBotStrategyName[i], j);
            InitializePlayersStatistics(LooseAggressiveBotStrategyName[i], j);
            InitializePlayersStatistics(ManiacBotStrategyName[i], j);
            InitializePlayersStatistics(UltraTightBotStrategyName[i], j);
        }
    }
    for (int j = 2; j <= MAX_NUMBER_OF_PLAYERS; j++)
    {
        InitializePlayersStatistics(HumanPlayerName[0], j);
    }
}

void SqliteLogStore::InitializePlayersStatistics(const string playerName, const int nbPlayers)
{

    sql += "INSERT OR REPLACE INTO PlayersStatistics (";
    sql += "player_name,nb_players";
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
            sql += "INSERT OR IGNORE INTO Ranking VALUES ('" + (*it_c)->getName() + "', 0, 0, 0);";
            const int lostStack = getIntegerValue((*it_c)->getName(), "Ranking", "lost_stack") + 1;
            sql += "UPDATE Ranking SET lost_stack = " + std::to_string(lostStack);
            sql += " WHERE player_name = '" + (*it_c)->getName() + "';";
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
                sql += "INSERT OR IGNORE INTO Ranking VALUES ('" + (*it_c)->getName() + "', 0, 0, 0);";
                const int wonGame = getIntegerValue((*it_c)->getName(), "Ranking", "won_game") + 1;
                sql += "UPDATE Ranking SET won_game = " + std::to_string(wonGame);
                sql += " WHERE player_name = '" + (*it_c)->getName() + "';";
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
        sql += "INSERT OR IGNORE INTO Ranking VALUES ('" + (*it_c)->getName() + "', 0, 0, 0);";
        const int playedGames = getIntegerValue((*it_c)->getName(), "Ranking", "played_games") + 1;
        sql += "UPDATE Ranking SET played_games = " + std::to_string(playedGames);
        sql += " WHERE player_name = '" + (*it_c)->getName() + "';";
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
            "SELECT " + attributeName + " FROM " + tableName + " WHERE player_name = \"" + playerName + "\"";
        if (sqlite3_get_table(mySqliteLogDb, sql_select.c_str(), &result_Player, &nRow_Player, &nCol_Player, &errmsg) !=
            SQLITE_OK)
        {
            cout << "Error in statement: " << sql_select.c_str() << "[" << errmsg << "]." << endl;
        }
        else
        {
            if (nRow_Player == 1)
            {
                result = atoi(result_Player[1]);
            }
            else
            {
                cout << "Warning : no data for player " << playerName << " in table " << tableName
                     << ". SQL query was :\n"
                     << sql_select << endl;
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
    sql += "player_name VARCHAR NOT NULL";
    sql += ",lost_stack LARGEINT";
    sql += ",won_game LARGEINT";
    sql += ",played_games LARGEINT";
    sql += ", PRIMARY KEY(player_name));";
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
            cout << "Error in statement: " << sql_select.c_str() << "[" << errmsg << "]." << endl;
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
        cout << "Error in statement: " << sql_transaction.c_str() << "[" << errmsg << "]." << endl;
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

        sql += " WHERE player_name = '" + (*it_c)->getName() + "' AND nb_players = " + std::to_string(i) + ";";

        exec_transaction();
    }
}

std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1> SqliteLogStore::getPlayerStatistics(const string& playerName)
{

    std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1> playerStatistics{};

    sqlite3* db;
    std::filesystem::path sqliteLogFileName;

    bool dirExists = std::filesystem::is_directory(myLogDir);

    // check if logging path exist
    if (myLogDir != "" && dirExists)
    {

        sqliteLogFileName.clear();
        sqliteLogFileName /= myLogDir;
        sqliteLogFileName /= string(SQL_LOG_FILE);

        // open sqlite-db
        int rc = sqlite3_open(sqliteLogFileName.string().c_str(), &db);

        if (rc)
        {
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return playerStatistics;
        }

        char* errmsg = 0;

        string sql_select = "SELECT * FROM PlayersStatistics WHERE player_name = '" + playerName + "';";

        sqlite3_stmt* stmt;

        rc = sqlite3_prepare_v2(db, sql_select.c_str(), -1, &stmt, 0);

        if (rc == SQLITE_OK)
        {

            int nCols = sqlite3_column_count(stmt);

            if (nCols)
            {

                while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
                {

                    int nbPlayers = 0;

                    for (int nCol = 0; nCol < nCols; nCol++)
                    {

                        const char* colName = sqlite3_column_name(stmt, nCol);

                        if (strcmp(colName, "nb_players") == 0)
                            nbPlayers = sqlite3_column_int(stmt, nCol);
                        else

                            // preflop
                            if (strcmp(colName, "pf_hands") == 0)
                                playerStatistics[nbPlayers].m_preflopStatistics.m_hands =
                                    sqlite3_column_int(stmt, nCol);
                            else if (strcmp(colName, "pf_folds") == 0)
                                playerStatistics[nbPlayers].m_preflopStatistics.m_folds =
                                    sqlite3_column_int(stmt, nCol);
                            else if (strcmp(colName, "pf_checks") == 0)
                                playerStatistics[nbPlayers].m_preflopStatistics.m_checks =
                                    sqlite3_column_int(stmt, nCol);
                            else if (strcmp(colName, "pf_calls") == 0)
                                playerStatistics[nbPlayers].m_preflopStatistics.m_calls =
                                    sqlite3_column_int(stmt, nCol);
                            else if (strcmp(colName, "pf_raises") == 0)
                                playerStatistics[nbPlayers].m_preflopStatistics.m_raises =
                                    sqlite3_column_int(stmt, nCol);
                            else if (strcmp(colName, "pf_limps") == 0)
                                playerStatistics[nbPlayers].m_preflopStatistics.m_limps =
                                    sqlite3_column_int(stmt, nCol);
                            else if (strcmp(colName, "pf_3Bets") == 0)
                                playerStatistics[nbPlayers].m_preflopStatistics.m_3Bets =
                                    sqlite3_column_int(stmt, nCol);
                            else if (strcmp(colName, "pf_call3Bets") == 0)
                                playerStatistics[nbPlayers].m_preflopStatistics.m_call3Bets =
                                    sqlite3_column_int(stmt, nCol);
                            else if (strcmp(colName, "pf_call3BetsOpportunities") == 0)
                                playerStatistics[nbPlayers].m_preflopStatistics.m_call3BetsOpportunities =
                                    sqlite3_column_int(stmt, nCol);
                            else if (strcmp(colName, "pf_4Bets") == 0)
                                playerStatistics[nbPlayers].m_preflopStatistics.m_4Bets =
                                    sqlite3_column_int(stmt, nCol);
                            else

                                // flop
                                if (strcmp(colName, "f_hands") == 0)
                                    playerStatistics[nbPlayers].m_flopStatistics.m_hands =
                                        sqlite3_column_int(stmt, nCol);
                                else if (strcmp(colName, "f_folds") == 0)
                                    playerStatistics[nbPlayers].m_flopStatistics.m_folds =
                                        sqlite3_column_int(stmt, nCol);
                                else if (strcmp(colName, "f_checks") == 0)
                                    playerStatistics[nbPlayers].m_flopStatistics.m_checks =
                                        sqlite3_column_int(stmt, nCol);
                                else if (strcmp(colName, "f_calls") == 0)
                                    playerStatistics[nbPlayers].m_flopStatistics.m_calls =
                                        sqlite3_column_int(stmt, nCol);
                                else if (strcmp(colName, "f_raises") == 0)
                                    playerStatistics[nbPlayers].m_flopStatistics.m_raises =
                                        sqlite3_column_int(stmt, nCol);
                                else if (strcmp(colName, "f_bets") == 0)
                                    playerStatistics[nbPlayers].m_flopStatistics.m_bets =
                                        sqlite3_column_int(stmt, nCol);
                                else if (strcmp(colName, "f_3Bets") == 0)
                                    playerStatistics[nbPlayers].m_flopStatistics.m_3Bets =
                                        sqlite3_column_int(stmt, nCol);
                                else if (strcmp(colName, "f_4Bets") == 0)
                                    playerStatistics[nbPlayers].m_flopStatistics.m_4Bets =
                                        sqlite3_column_int(stmt, nCol);
                                else if (strcmp(colName, "f_continuationBets") == 0)
                                    playerStatistics[nbPlayers].m_flopStatistics.m_continuationBets =
                                        sqlite3_column_int(stmt, nCol);
                                else if (strcmp(colName, "f_continuationBetsOpportunities") == 0)
                                    playerStatistics[nbPlayers].m_flopStatistics.m_continuationBetsOpportunities =
                                        sqlite3_column_int(stmt, nCol);
                                else

                                    // turn
                                    if (strcmp(colName, "t_hands") == 0)
                                        playerStatistics[nbPlayers].m_turnStatistics.m_hands =
                                            sqlite3_column_int(stmt, nCol);
                                    else if (strcmp(colName, "t_folds") == 0)
                                        playerStatistics[nbPlayers].m_turnStatistics.m_folds =
                                            sqlite3_column_int(stmt, nCol);
                                    else if (strcmp(colName, "t_checks") == 0)
                                        playerStatistics[nbPlayers].m_turnStatistics.m_checks =
                                            sqlite3_column_int(stmt, nCol);
                                    else if (strcmp(colName, "t_calls") == 0)
                                        playerStatistics[nbPlayers].m_turnStatistics.m_calls =
                                            sqlite3_column_int(stmt, nCol);
                                    else if (strcmp(colName, "t_raises") == 0)
                                        playerStatistics[nbPlayers].m_turnStatistics.m_raises =
                                            sqlite3_column_int(stmt, nCol);
                                    else if (strcmp(colName, "t_bets") == 0)
                                        playerStatistics[nbPlayers].m_turnStatistics.m_bets =
                                            sqlite3_column_int(stmt, nCol);
                                    else if (strcmp(colName, "t_3Bets") == 0)
                                        playerStatistics[nbPlayers].m_turnStatistics.m_3Bets =
                                            sqlite3_column_int(stmt, nCol);
                                    else if (strcmp(colName, "t_4Bets") == 0)
                                        playerStatistics[nbPlayers].m_turnStatistics.m_4Bets =
                                            sqlite3_column_int(stmt, nCol);
                                    else

                                        // river
                                        if (strcmp(colName, "r_hands") == 0)
                                            playerStatistics[nbPlayers].m_riverStatistics.m_hands =
                                                sqlite3_column_int(stmt, nCol);
                                        else if (strcmp(colName, "r_folds") == 0)
                                            playerStatistics[nbPlayers].m_riverStatistics.m_folds =
                                                sqlite3_column_int(stmt, nCol);
                                        else if (strcmp(colName, "r_checks") == 0)
                                            playerStatistics[nbPlayers].m_riverStatistics.m_checks =
                                                sqlite3_column_int(stmt, nCol);
                                        else if (strcmp(colName, "r_calls") == 0)
                                            playerStatistics[nbPlayers].m_riverStatistics.m_calls =
                                                sqlite3_column_int(stmt, nCol);
                                        else if (strcmp(colName, "r_raises") == 0)
                                            playerStatistics[nbPlayers].m_riverStatistics.m_raises =
                                                sqlite3_column_int(stmt, nCol);
                                        else if (strcmp(colName, "r_bets") == 0)
                                            playerStatistics[nbPlayers].m_riverStatistics.m_bets =
                                                sqlite3_column_int(stmt, nCol);
                                        else if (strcmp(colName, "r_3Bets") == 0)
                                            playerStatistics[nbPlayers].m_riverStatistics.m_3Bets =
                                                sqlite3_column_int(stmt, nCol);
                                        else if (strcmp(colName, "r_4Bets") == 0)
                                            playerStatistics[nbPlayers].m_riverStatistics.m_4Bets =
                                                sqlite3_column_int(stmt, nCol);
                    }
                }
            }
            sqlite3_finalize(stmt);
        }
        sqlite3_close(db);
    }

    return playerStatistics;
}
} // namespace pkt::infra
