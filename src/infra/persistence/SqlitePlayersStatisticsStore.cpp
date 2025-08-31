// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "SqlitePlayersStatisticsStore.h"
#include <core/player/typedefs.h>
#include "core/engine/CardUtilities.h"
#include "core/engine/EngineDefs.h"
#include "core/interfaces/NullLogger.h"
#include "core/player/PlayerStatistics.h"
#include "core/player/deprecated/HumanPlayer.h"
#include "core/player/deprecated/Player.h"
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

SqlitePlayersStatisticsStore::SqlitePlayersStatisticsStore(std::shared_ptr<SqliteDb> db) : myDb(db)

{
}

SqlitePlayersStatisticsStore::~SqlitePlayersStatisticsStore()
{
}

void SqlitePlayersStatisticsStore::initializeStrategyStatistics(const string& playerName, const int nbPlayers)
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
}

void SqlitePlayersStatisticsStore::updatePlayersStatistics(PlayerList seatsList)
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

        mySql += " WHERE strategy_name = '" + (*itC)->getName() + "' AND nb_players = " + std::to_string(i) + ";";
    }
}

std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1>
SqlitePlayersStatisticsStore::getPlayerStatistics(const std::string& playerName)
{
    // temporary stub
    std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1> playerStatistics{};

    return playerStatistics;
}
void SqlitePlayersStatisticsStore::updatePlayerStatistics(PlayerStatistics& stats, const std::string& columnName,
                                                          sqlite3_stmt* stmt, int nCol) const
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
