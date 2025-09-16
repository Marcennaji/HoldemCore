#include "SqlitePlayersStatisticsStore.h"
#include "SqliteDb.h"
#include "SqliteStatement.h"
#include "core/player/Player.h"

#include <sstream>
#include <stdexcept>

namespace pkt::infra
{

using namespace pkt::core;
using namespace pkt::core::player;

SqlitePlayersStatisticsStore::SqlitePlayersStatisticsStore(std::unique_ptr<SqliteDb> db) : myDb(std::move(db))
{
    if (!myDb)
    {
        throw std::invalid_argument("SqlitePlayersStatisticsStore requires a valid SqliteDb");
    }

    // Ensure schema exists
    myDb->exec(R"SQL(
        CREATE TABLE IF NOT EXISTS PlayersStatistics (
            strategy_name TEXT NOT NULL,
            nb_players INTEGER NOT NULL,
            /* preflop */
            pf_hands INTEGER NOT NULL,
            pf_checks INTEGER NOT NULL,
            pf_calls INTEGER NOT NULL,
            pf_raises INTEGER NOT NULL,
            pf_threeBets INTEGER NOT NULL,
            pf_callthreeBets INTEGER NOT NULL,
            pf_callthreeBetsOpportunities INTEGER NOT NULL,
            pf_fourBets INTEGER NOT NULL,
            pf_folds INTEGER NOT NULL,
            pf_limps INTEGER NOT NULL,
            /* flop */
            f_hands INTEGER NOT NULL,
            f_checks INTEGER NOT NULL,
            f_bets INTEGER NOT NULL,
            f_calls INTEGER NOT NULL,
            f_raises INTEGER NOT NULL,
            f_threeBets INTEGER NOT NULL,
            f_fourBets INTEGER NOT NULL,
            f_folds INTEGER NOT NULL,
            f_continuationBets INTEGER NOT NULL,
            f_continuationBetsOpportunities INTEGER NOT NULL,
            /* turn */
            t_hands INTEGER NOT NULL,
            t_checks INTEGER NOT NULL,
            t_bets INTEGER NOT NULL,
            t_calls INTEGER NOT NULL,
            t_raises INTEGER NOT NULL,
            t_threeBets INTEGER NOT NULL,
            t_fourBets INTEGER NOT NULL,
            t_folds INTEGER NOT NULL,
            /* river */
            r_hands INTEGER NOT NULL,
            r_checks INTEGER NOT NULL,
            r_bets INTEGER NOT NULL,
            r_calls INTEGER NOT NULL,
            r_raises INTEGER NOT NULL,
            r_threeBets INTEGER NOT NULL,
            r_fourBets INTEGER NOT NULL,
            r_folds INTEGER NOT NULL,
            PRIMARY KEY (strategy_name, nb_players)
        );
    )SQL");
}

SqlitePlayersStatisticsStore::~SqlitePlayersStatisticsStore() = default;

void SqlitePlayersStatisticsStore::initializeStrategyStatistics(const std::string& playerName, const int nbPlayers)
{
    // Insert if not exists
    auto stmt = myDb->prepare("INSERT OR IGNORE INTO PlayersStatistics("
                              "strategy_name, nb_players,"
                              "pf_hands,pf_checks,pf_calls,pf_raises,pf_threeBets,pf_callthreeBets,pf_"
                              "callthreeBetsOpportunities,pf_fourBets,pf_folds,pf_limps,"
                              "f_hands,f_checks,f_bets,f_calls,f_raises,f_threeBets,f_fourBets,f_folds,f_"
                              "continuationBets,f_continuationBetsOpportunities,"
                              "t_hands,t_checks,t_bets,t_calls,t_raises,t_threeBets,t_fourBets,t_folds,"
                              "r_hands,r_checks,r_bets,r_calls,r_raises,r_threeBets,r_fourBets,r_folds"
                              ") VALUES (?1, ?2,"
                              // 35 zeros:
                              "0,0,0,0,0,0,0,0,0,0,"
                              "0,0,0,0,0,0,0,0,0,0,"
                              "0,0,0,0,0,0,0,0,"
                              "0,0,0,0,0,0,0,0)");
    stmt->bindText(1, playerName);
    stmt->bindInt(2, nbPlayers);
    stmt->step();
}

void SqlitePlayersStatisticsStore::savePlayersStatistics(PlayerList seatsList)
{
    int nbPlayers = static_cast<int>(seatsList->size());

    for (auto& player : *seatsList)
    {
        const auto& stats = player->getStatisticsUpdater()->getStatistics(nbPlayers);
        if (stats.preflopStatistics.hands == 0)
        {
            continue;
        }

        initializeStrategyStatistics(player->getName(), nbPlayers);

        auto stmt = myDb->prepare(
            "UPDATE PlayersStatistics SET "
            "pf_hands=?3,pf_checks=?4,pf_calls=?5,pf_raises=?6,pf_threeBets=?7,pf_callthreeBets=?8,"
            "pf_callthreeBetsOpportunities=?9,pf_fourBets=?10,pf_folds=?11,pf_limps=?12,"
            "f_hands=?13,f_checks=?14,f_bets=?15,f_calls=?16,f_raises=?17,f_threeBets=?18,f_fourBets=?19,f_folds=?20,"
            "f_continuationBets=?21,f_continuationBetsOpportunities=?22,"
            "t_hands=?23,t_checks=?24,t_bets=?25,t_calls=?26,t_raises=?27,t_threeBets=?28,t_fourBets=?29,t_folds=?30,"
            "r_hands=?31,r_checks=?32,r_bets=?33,r_calls=?34,r_raises=?35,r_threeBets=?36,r_fourBets=?37,r_folds=?38 "
            "WHERE strategy_name=?1 AND nb_players=?2");

        // keys
        stmt->bindText(1, player->getName());
        stmt->bindInt(2, nbPlayers);

        // preflop
        stmt->bindInt(3, stats.preflopStatistics.hands);
        stmt->bindInt(4, stats.preflopStatistics.checks);
        stmt->bindInt(5, stats.preflopStatistics.calls);
        stmt->bindInt(6, stats.preflopStatistics.raises);
        stmt->bindInt(7, stats.preflopStatistics.threeBets);
        stmt->bindInt(8, stats.preflopStatistics.callthreeBets);
        stmt->bindInt(9, stats.preflopStatistics.callthreeBetsOpportunities);
        stmt->bindInt(10, stats.preflopStatistics.fourBets);
        stmt->bindInt(11, stats.preflopStatistics.folds);
        stmt->bindInt(12, stats.preflopStatistics.limps);

        // flop
        stmt->bindInt(13, stats.flopStatistics.hands);
        stmt->bindInt(14, stats.flopStatistics.checks);
        stmt->bindInt(15, stats.flopStatistics.bets);
        stmt->bindInt(16, stats.flopStatistics.calls);
        stmt->bindInt(17, stats.flopStatistics.raises);
        stmt->bindInt(18, stats.flopStatistics.threeBets);
        stmt->bindInt(19, stats.flopStatistics.fourBets);
        stmt->bindInt(20, stats.flopStatistics.folds);
        stmt->bindInt(21, stats.flopStatistics.continuationBets);
        stmt->bindInt(22, stats.flopStatistics.continuationBetsOpportunities);

        // turn
        stmt->bindInt(23, stats.turnStatistics.hands);
        stmt->bindInt(24, stats.turnStatistics.checks);
        stmt->bindInt(25, stats.turnStatistics.bets);
        stmt->bindInt(26, stats.turnStatistics.calls);
        stmt->bindInt(27, stats.turnStatistics.raises);
        stmt->bindInt(28, stats.turnStatistics.threeBets);
        stmt->bindInt(29, stats.turnStatistics.fourBets);
        stmt->bindInt(30, stats.turnStatistics.folds);

        // river
        stmt->bindInt(31, stats.riverStatistics.hands);
        stmt->bindInt(32, stats.riverStatistics.checks);
        stmt->bindInt(33, stats.riverStatistics.bets);
        stmt->bindInt(34, stats.riverStatistics.calls);
        stmt->bindInt(35, stats.riverStatistics.raises);
        stmt->bindInt(36, stats.riverStatistics.threeBets);
        stmt->bindInt(37, stats.riverStatistics.fourBets);
        stmt->bindInt(38, stats.riverStatistics.folds);

        stmt->step();
    }
}

std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1>
SqlitePlayersStatisticsStore::loadPlayerStatistics(const std::string& playerName)
{
    // make sure that we have initial values for this player
    for (int nbPlayers = 2; nbPlayers <= MAX_NUMBER_OF_PLAYERS; nbPlayers++)
        initializeStrategyStatistics(playerName, nbPlayers);

    std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1> results{};

    auto stmt = myDb->prepare("SELECT nb_players,"
                              "pf_hands,pf_checks,pf_calls,pf_raises,pf_threeBets,pf_callthreeBets,pf_"
                              "callthreeBetsOpportunities,pf_fourBets,pf_folds,pf_limps,"
                              "f_hands,f_checks,f_bets,f_calls,f_raises,f_threeBets,f_fourBets,f_folds,f_"
                              "continuationBets,f_continuationBetsOpportunities,"
                              "t_hands,t_checks,t_bets,t_calls,t_raises,t_threeBets,t_fourBets,t_folds,"
                              "r_hands,r_checks,r_bets,r_calls,r_raises,r_threeBets,r_fourBets,r_folds "
                              "FROM PlayersStatistics WHERE strategy_name=?1");
    stmt->bindText(1, playerName);

    while (stmt->step())
    {
        int n = stmt->getInt(0);
        if (n < 0 || n > MAX_NUMBER_OF_PLAYERS)
            continue;

        PlayerStatistics ps{};

        // map preflop
        ps.preflopStatistics.hands = stmt->getInt(1);
        ps.preflopStatistics.checks = stmt->getInt(2);
        ps.preflopStatistics.calls = stmt->getInt(3);
        ps.preflopStatistics.raises = stmt->getInt(4);
        ps.preflopStatistics.threeBets = stmt->getInt(5);
        ps.preflopStatistics.callthreeBets = stmt->getInt(6);
        ps.preflopStatistics.callthreeBetsOpportunities = stmt->getInt(7);
        ps.preflopStatistics.fourBets = stmt->getInt(8);
        ps.preflopStatistics.folds = stmt->getInt(9);
        ps.preflopStatistics.limps = stmt->getInt(10);

        // flop
        ps.flopStatistics.hands = stmt->getInt(11);
        ps.flopStatistics.checks = stmt->getInt(12);
        ps.flopStatistics.bets = stmt->getInt(13);
        ps.flopStatistics.calls = stmt->getInt(14);
        ps.flopStatistics.raises = stmt->getInt(15);
        ps.flopStatistics.threeBets = stmt->getInt(16);
        ps.flopStatistics.fourBets = stmt->getInt(17);
        ps.flopStatistics.folds = stmt->getInt(18);
        ps.flopStatistics.continuationBets = stmt->getInt(19);
        ps.flopStatistics.continuationBetsOpportunities = stmt->getInt(20);

        // turn
        ps.turnStatistics.hands = stmt->getInt(21);
        ps.turnStatistics.checks = stmt->getInt(22);
        ps.turnStatistics.bets = stmt->getInt(23);
        ps.turnStatistics.calls = stmt->getInt(24);
        ps.turnStatistics.raises = stmt->getInt(25);
        ps.turnStatistics.threeBets = stmt->getInt(26);
        ps.turnStatistics.fourBets = stmt->getInt(27);
        ps.turnStatistics.folds = stmt->getInt(28);

        // river
        ps.riverStatistics.hands = stmt->getInt(29);
        ps.riverStatistics.checks = stmt->getInt(30);
        ps.riverStatistics.bets = stmt->getInt(31);
        ps.riverStatistics.calls = stmt->getInt(32);
        ps.riverStatistics.raises = stmt->getInt(33);
        ps.riverStatistics.threeBets = stmt->getInt(34);
        ps.riverStatistics.fourBets = stmt->getInt(35);
        ps.riverStatistics.folds = stmt->getInt(36);

        results[n] = ps;
    }

    return results;
}

} // namespace pkt::infra
