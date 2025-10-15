#include "SqlitePlayersStatisticsStore.h"
#include "SqliteDb.h"
#include "SqliteStatement.h"
#include "core/player/Player.h"
#include "core/player/strategy/PlayerStrategy.h"

#include <sstream>
#include <stdexcept>

namespace pkt::infra
{

using namespace pkt::core;
using namespace pkt::core::player;

SqlitePlayersStatisticsStore::SqlitePlayersStatisticsStore(std::unique_ptr<SqliteDb> db) : m_db(std::move(db))
{
    if (!m_db)
    {
        throw std::invalid_argument("SqlitePlayersStatisticsStore requires a valid SqliteDb");
    }

    // Ensure schema exists
    m_db->exec(R"SQL(
        CREATE TABLE  IF NOT EXISTS PlayersStatistics (
            strategy_name TEXT NOT NULL,
            table_type TEXT NOT NULL CHECK(table_type IN ('HU', 'SH', 'FR')),

            /* preflop */
            pf_hands INTEGER NOT NULL,
            pf_checks INTEGER NOT NULL,
            pf_calls INTEGER NOT NULL,
            pf_raises INTEGER NOT NULL,
            pf_threeBets INTEGER NOT NULL,
            pf_fourBets INTEGER NOT NULL,
            pf_folds INTEGER NOT NULL,
            pf_limps INTEGER NOT NULL,
            pf_callThreeBets INTEGER NOT NULL,
            pf_callThreeBetsOpportunities INTEGER NOT NULL,

            /* flop */
            f_hands INTEGER NOT NULL,
            f_checks INTEGER NOT NULL,
            f_bets INTEGER NOT NULL,
            f_calls INTEGER NOT NULL,
            f_raises INTEGER NOT NULL,
            f_folds INTEGER NOT NULL,
            f_continuationBets INTEGER NOT NULL,
            f_continuationBetsOpportunities INTEGER NOT NULL,

            /* turn */
            t_hands INTEGER NOT NULL,
            t_checks INTEGER NOT NULL,
            t_bets INTEGER NOT NULL,
            t_calls INTEGER NOT NULL,
            t_raises INTEGER NOT NULL,
            t_folds INTEGER NOT NULL,

            /* river */
            r_hands INTEGER NOT NULL,
            r_checks INTEGER NOT NULL,
            r_bets INTEGER NOT NULL,
            r_calls INTEGER NOT NULL,
            r_raises INTEGER NOT NULL,
            r_folds INTEGER NOT NULL,

            /* showdown */
            sd_wentToShowdown INTEGER NOT NULL,
            sd_wonShowdown INTEGER NOT NULL,
            sd_wonWithoutShowdown INTEGER NOT NULL,

            /* bet sizing */
            avg_bet_size_ratio REAL NOT NULL DEFAULT 0.0,

            PRIMARY KEY (strategy_name, table_type)
        );
    )SQL");
}

SqlitePlayersStatisticsStore::~SqlitePlayersStatisticsStore() = default;

std::string SqlitePlayersStatisticsStore::getTableType(const int nbPlayers)
{
    if (nbPlayers == 2)
        return "HU"; // Heads-Up
    if (nbPlayers <= 6)
        return "SH"; // Short-Handed
    return "FR";     // Full Ring
}

void SqlitePlayersStatisticsStore::initializeStrategyStatistics(const std::string& strategyName, const int nbPlayers)
{
    std::string tableType = getTableType(nbPlayers);

    // Insert if not exists
    auto stmt = m_db->prepare("INSERT OR IGNORE INTO PlayersStatistics("
                              "strategy_name, table_type,"
                              "pf_hands,pf_checks,pf_calls,pf_raises,pf_threeBets,pf_callThreeBets,pf_"
                              "callThreeBetsOpportunities,pf_fourBets,pf_folds,pf_limps,"
                              "f_hands,f_checks,f_bets,f_calls,f_raises,f_folds,f_"
                              "continuationBets,f_continuationBetsOpportunities,"
                              "t_hands,t_checks,t_bets,t_calls,t_raises,t_folds,"
                              "r_hands,r_checks,r_bets,r_calls,r_raises,r_folds,"
                              "sd_wentToShowdown,sd_wonShowdown,sd_wonWithoutShowdown,"
                              "avg_bet_size_ratio"
                              ") VALUES (?1, ?2,"
                              // pf (10) + f (8) + t (6) + r (6) + sd (3) + avg (1) = 34 values
                              "0,0,0,0,0,0,0,0,0,0," // preflop (10)
                              "0,0,0,0,0,0,0,0,"     // flop (8)
                              "0,0,0,0,0,0,"         // turn (6)
                              "0,0,0,0,0,0,"         // river (6)
                              "0,0,0,"               // showdown (3)
                              "0.0)");               // avg_bet_size_ratio
    stmt->bindText(1, strategyName);
    stmt->bindText(2, tableType);
    stmt->step();
}

void SqlitePlayersStatisticsStore::savePlayersStatistics(PlayerList seatsList)
{
    int nbPlayers = static_cast<int>(seatsList->size());
    std::string tableType = getTableType(nbPlayers);

    for (auto& player : *seatsList)
    {
        std::string strategyName = player->getStrategyName();

        // Skip players without a valid strategy
        if (strategyName.empty() || strategyName == PlayerStrategy::NO_STRATEGY_NAME)
        {
            continue;
        }

        // Get delta since last save
        const auto delta = player->getStatisticsUpdater()->getStatisticsDeltaAndUpdateBaseline(nbPlayers);
        if (delta.preflopStatistics.hands == 0)
        {
            continue; // No new hands played, skip saving
        }

        initializeStrategyStatistics(strategyName, nbPlayers);

        auto stmt =
            m_db->prepare("UPDATE PlayersStatistics SET "
                          "pf_hands=pf_hands+?3,pf_checks=pf_checks+?4,pf_calls=pf_calls+?5,"
                          "pf_raises=pf_raises+?6,pf_threeBets=pf_threeBets+?7,pf_callThreeBets=pf_callThreeBets+?8,"
                          "pf_callThreeBetsOpportunities=pf_callThreeBetsOpportunities+?9,"
                          "pf_fourBets=pf_fourBets+?10,pf_folds=pf_folds+?11,pf_limps=pf_limps+?12,"
                          "f_hands=f_hands+?13,f_checks=f_checks+?14,f_bets=f_bets+?15,f_calls=f_calls+?16,"
                          "f_raises=f_raises+?17,f_folds=f_folds+?18,"
                          "f_continuationBets=f_continuationBets+?19,"
                          "f_continuationBetsOpportunities=f_continuationBetsOpportunities+?20,"
                          "t_hands=t_hands+?21,t_checks=t_checks+?22,t_bets=t_bets+?23,t_calls=t_calls+?24,"
                          "t_raises=t_raises+?25,t_folds=t_folds+?26,"
                          "r_hands=r_hands+?27,r_checks=r_checks+?28,r_bets=r_bets+?29,r_calls=r_calls+?30,"
                          "r_raises=r_raises+?31,r_folds=r_folds+?32,"
                          "sd_wentToShowdown=sd_wentToShowdown+?33,sd_wonShowdown=sd_wonShowdown+?34,"
                          "sd_wonWithoutShowdown=sd_wonWithoutShowdown+?35,"
                          "avg_bet_size_ratio=?36 "
                          "WHERE strategy_name=?1 AND table_type=?2");

        // keys
        stmt->bindText(1, strategyName);
        stmt->bindText(2, tableType);

        // preflop
        stmt->bindInt(3, delta.preflopStatistics.hands);
        stmt->bindInt(4, delta.preflopStatistics.checks);
        stmt->bindInt(5, delta.preflopStatistics.calls);
        stmt->bindInt(6, delta.preflopStatistics.raises);
        stmt->bindInt(7, delta.preflopStatistics.threeBets);
        stmt->bindInt(8, delta.preflopStatistics.callthreeBets);
        stmt->bindInt(9, delta.preflopStatistics.callthreeBetsOpportunities);
        stmt->bindInt(10, delta.preflopStatistics.fourBets);
        stmt->bindInt(11, delta.preflopStatistics.folds);
        stmt->bindInt(12, delta.preflopStatistics.limps);

        // flop
        stmt->bindInt(13, delta.flopStatistics.hands);
        stmt->bindInt(14, delta.flopStatistics.checks);
        stmt->bindInt(15, delta.flopStatistics.bets);
        stmt->bindInt(16, delta.flopStatistics.calls);
        stmt->bindInt(17, delta.flopStatistics.raises);
        stmt->bindInt(18, delta.flopStatistics.folds);
        stmt->bindInt(19, delta.flopStatistics.continuationBets);
        stmt->bindInt(20, delta.flopStatistics.continuationBetsOpportunities);

        // turn
        stmt->bindInt(21, delta.turnStatistics.hands);
        stmt->bindInt(22, delta.turnStatistics.checks);
        stmt->bindInt(23, delta.turnStatistics.bets);
        stmt->bindInt(24, delta.turnStatistics.calls);
        stmt->bindInt(25, delta.turnStatistics.raises);
        stmt->bindInt(26, delta.turnStatistics.folds);

        // river
        stmt->bindInt(27, delta.riverStatistics.hands);
        stmt->bindInt(28, delta.riverStatistics.checks);
        stmt->bindInt(29, delta.riverStatistics.bets);
        stmt->bindInt(30, delta.riverStatistics.calls);
        stmt->bindInt(31, delta.riverStatistics.raises);
        stmt->bindInt(32, delta.riverStatistics.folds);

        // showdown (using placeholder values if not available in stats)
        stmt->bindInt(33, 0); // TODO: Add sd_wentToShowdown to PlayerStatistics
        stmt->bindInt(34, 0); // TODO: Add sd_wonShowdown to PlayerStatistics
        stmt->bindInt(35, 0); // TODO: Add sd_wonWithoutShowdown to PlayerStatistics

        // bet sizing (using placeholder if not available)
        stmt->bindDouble(36, 0.0); // TODO: Add avg_bet_size_ratio to PlayerStatistics

        stmt->step();
    }
}

std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1>
SqlitePlayersStatisticsStore::loadPlayerStatistics(const std::string& strategyName)
{
    // Skip loading for invalid strategy names
    if (strategyName.empty() || strategyName == PlayerStrategy::NO_STRATEGY_NAME)
    {
        return std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1>{};
    }

    // make sure that we have initial values for this player
    for (int nbPlayers = 2; nbPlayers <= MAX_NUMBER_OF_PLAYERS; nbPlayers++)
        initializeStrategyStatistics(strategyName, nbPlayers);

    std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1> results{};

    auto stmt = m_db->prepare("SELECT table_type,"
                              "pf_hands,pf_checks,pf_calls,pf_raises,pf_threeBets,pf_callThreeBets,pf_"
                              "callThreeBetsOpportunities,pf_fourBets,pf_folds,pf_limps,"
                              "f_hands,f_checks,f_bets,f_calls,f_raises,f_folds,f_"
                              "continuationBets,f_continuationBetsOpportunities,"
                              "t_hands,t_checks,t_bets,t_calls,t_raises,t_folds,"
                              "r_hands,r_checks,r_bets,r_calls,r_raises,r_folds,"
                              "sd_wentToShowdown,sd_wonShowdown,sd_wonWithoutShowdown,"
                              "avg_bet_size_ratio "
                              "FROM PlayersStatistics WHERE strategy_name=?1");
    stmt->bindText(1, strategyName);

    while (stmt->step())
    {
        std::string tableType = stmt->getText(0);

        // Map table_type back to nbPlayers for array indexing
        int nbPlayers = 0;
        if (tableType == "HU")
            nbPlayers = 2;
        else if (tableType == "SH")
            nbPlayers = 6; // Use 6 as representative for SH (3-6 players)
        else if (tableType == "FR")
            nbPlayers = 9; // Use 9 as representative for FR (7-10 players)

        if (nbPlayers < 2 || nbPlayers > MAX_NUMBER_OF_PLAYERS)
            continue;

        PlayerStatistics ps{};

        // preflop
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
        ps.flopStatistics.folds = stmt->getInt(16);
        ps.flopStatistics.continuationBets = stmt->getInt(17);
        ps.flopStatistics.continuationBetsOpportunities = stmt->getInt(18);

        // turn
        ps.turnStatistics.hands = stmt->getInt(19);
        ps.turnStatistics.checks = stmt->getInt(20);
        ps.turnStatistics.bets = stmt->getInt(21);
        ps.turnStatistics.calls = stmt->getInt(22);
        ps.turnStatistics.raises = stmt->getInt(23);
        ps.turnStatistics.folds = stmt->getInt(24);

        // river
        ps.riverStatistics.hands = stmt->getInt(25);
        ps.riverStatistics.checks = stmt->getInt(26);
        ps.riverStatistics.bets = stmt->getInt(27);
        ps.riverStatistics.calls = stmt->getInt(28);
        ps.riverStatistics.raises = stmt->getInt(29);
        ps.riverStatistics.folds = stmt->getInt(30);

        // showdown - TODO: Add these to PlayerStatistics struct
        // int sd_wentToShowdown = stmt->getInt(31);
        // int sd_wonShowdown = stmt->getInt(32);
        // int sd_wonWithoutShowdown = stmt->getInt(33);

        // bet sizing - TODO: Add this to PlayerStatistics struct
        // double avg_bet_size_ratio = stmt->getDouble(34);

        // Store in result array - use same stats for all player counts that map to this table_type
        if (tableType == "HU")
        {
            results[2] = ps;
        }
        else if (tableType == "SH")
        {
            for (int i = 3; i <= 6; i++)
                results[i] = ps;
        }
        else if (tableType == "FR")
        {
            for (int i = 7; i <= MAX_NUMBER_OF_PLAYERS; i++)
                results[i] = ps;
        }
    }

    return results;
}

} // namespace pkt::infra
