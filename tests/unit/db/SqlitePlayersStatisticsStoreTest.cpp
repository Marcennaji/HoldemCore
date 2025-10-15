#include "SqlitePlayersStatisticsStoreTest.h"
#include "adapters/infrastructure/statistics/NullPlayersStatisticsStore.h"
#include "common/DeterministicStrategy.h"
#include "common/EngineTest.h"
#include "common/FakeRandomizer.h"
#include "core/engine/EngineDefs.h"
#include "core/engine/state/PreflopState.h"
#include "core/player/Helpers.h"

#include "adapters/infrastructure/hand_evaluation/PsimHandEvaluationEngine.h"
#include "adapters/infrastructure/logger/ConsoleLogger.h"
#include "adapters/infrastructure/statistics/sqlite/SqliteDb.h"

#include <memory>

using namespace pkt::core;
using namespace pkt::core::player;
using namespace pkt::infra;

using namespace std;
namespace pkt::test
{

void SqlitePlayersStatisticsStoreTest::SetUp()
{
    EngineTest::SetUp();

    auto db = std::make_unique<pkt::infra::SqliteDb>(":memory:");
    m_playersStatisticsStore = std::make_unique<pkt::infra::SqlitePlayersStatisticsStore>(std::move(db));

    auto logger = std::make_shared<pkt::infra::ConsoleLogger>();
    logger->setLogLevel(getTestLogLevel());
    m_logger = logger;

    auto randomizer = std::make_shared<FakeRandomizer>();
    randomizer->values = {3, 5, 7};
    m_randomizer = randomizer;

    m_factory = std::make_unique<EngineFactory>(m_events, *m_logger, *m_handEvaluationEngine, *m_playersStatisticsStore,
                                                *m_randomizer);

    gameData.maxNumberOfPlayers = MAX_NUMBER_OF_PLAYERS;
    gameData.startMoney = 1000;
    gameData.firstSmallBlind = 10;
    gameData.tableProfile = TableProfile::RandomOpponents;
}

void SqlitePlayersStatisticsStoreTest::TearDown()
{
    EngineTest::TearDown();
}

// Test that statistics accumulate correctly across multiple hands (delta tracking)
TEST_F(SqlitePlayersStatisticsStoreTest, MultipleHandsAccumulateCorrectly)
{
    auto& store = *m_playersStatisticsStore;
    int nbPlayers = 3;

    // Play first hand
    setupHandWithoutInitialize(nbPlayers, gameData);
    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    auto dealerStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    dealerStrategy->setLastAction(pkt::core::GameState::Preflop, {playerDealer->getId(), pkt::core::ActionType::Fold});
    playerDealer->setStrategy(std::move(dealerStrategy));

    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerSb->getId(), pkt::core::ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    playerBb->setStrategy(std::move(bbStrategy));

    m_hand->initialize();
    m_hand->dealHoleCards(0);

    auto* preflop = dynamic_cast<pkt::core::PreflopState*>(&m_hand->getState());
    ASSERT_NE(preflop, nullptr);

    preflop->promptPlayerAction(*m_hand, *playerDealer);
    preflop->promptPlayerAction(*m_hand, *playerSb);

    // After first hand: 2 hands, 2 folds
    auto statsAfterHand1 = store.loadPlayerStatistics("DeterministicStrategy");
    EXPECT_EQ(statsAfterHand1[nbPlayers].preflopStatistics.hands, 2);
    EXPECT_EQ(statsAfterHand1[nbPlayers].preflopStatistics.folds, 2);

    // Play second hand with same players
    setupHandWithoutInitialize(nbPlayers, gameData);
    playerDealer = getPlayerById(m_actingPlayersList, 0);
    playerSb = getPlayerById(m_actingPlayersList, 1);
    playerBb = getPlayerById(m_actingPlayersList, 2);

    dealerStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    dealerStrategy->setLastAction(pkt::core::GameState::Preflop, {playerDealer->getId(), pkt::core::ActionType::Fold});
    playerDealer->setStrategy(std::move(dealerStrategy));

    sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerSb->getId(), pkt::core::ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    playerBb->setStrategy(std::move(bbStrategy));

    m_hand->initialize();
    m_hand->dealHoleCards(0);

    preflop = dynamic_cast<pkt::core::PreflopState*>(&m_hand->getState());
    ASSERT_NE(preflop, nullptr);

    preflop->promptPlayerAction(*m_hand, *playerDealer);
    preflop->promptPlayerAction(*m_hand, *playerSb);

    // After second hand: 4 hands, 4 folds (NOT 6 hands due to triangular growth!)
    auto statsAfterHand2 = store.loadPlayerStatistics("DeterministicStrategy");
    EXPECT_EQ(statsAfterHand2[nbPlayers].preflopStatistics.hands, 4);
    EXPECT_EQ(statsAfterHand2[nbPlayers].preflopStatistics.folds, 4);

    // Play third hand
    setupHandWithoutInitialize(nbPlayers, gameData);
    playerDealer = getPlayerById(m_actingPlayersList, 0);
    playerSb = getPlayerById(m_actingPlayersList, 1);
    playerBb = getPlayerById(m_actingPlayersList, 2);

    dealerStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    dealerStrategy->setLastAction(pkt::core::GameState::Preflop, {playerDealer->getId(), pkt::core::ActionType::Fold});
    playerDealer->setStrategy(std::move(dealerStrategy));

    sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerSb->getId(), pkt::core::ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    playerBb->setStrategy(std::move(bbStrategy));

    m_hand->initialize();
    m_hand->dealHoleCards(0);

    preflop = dynamic_cast<pkt::core::PreflopState*>(&m_hand->getState());
    ASSERT_NE(preflop, nullptr);

    preflop->promptPlayerAction(*m_hand, *playerDealer);
    preflop->promptPlayerAction(*m_hand, *playerSb);

    // After third hand: 6 hands, 6 folds (NOT 12 hands!)
    auto statsAfterHand3 = store.loadPlayerStatistics("DeterministicStrategy");
    EXPECT_EQ(statsAfterHand3[nbPlayers].preflopStatistics.hands, 6);
    EXPECT_EQ(statsAfterHand3[nbPlayers].preflopStatistics.folds, 6);
}

// Test that street-specific statistics count correctly (no double-counting)
TEST_F(SqlitePlayersStatisticsStoreTest, StreetStatisticsNoDuplicates)
{
    auto& store = *m_playersStatisticsStore;
    int nbPlayers = 2;

    // Play hand where one player sees all streets
    setupHandWithoutInitialize(nbPlayers, gameData);
    auto player1 = getPlayerById(m_actingPlayersList, 0);
    auto player2 = getPlayerById(m_actingPlayersList, 1);

    // Player 1 folds preflop, Player 2 sees all streets (but won't act post-flop in this simplified test)
    auto strategy1 = std::make_unique<pkt::test::DeterministicStrategy>();
    strategy1->setLastAction(pkt::core::GameState::Preflop, {player1->getId(), pkt::core::ActionType::Fold});
    player1->setStrategy(std::move(strategy1));

    auto strategy2 = std::make_unique<pkt::test::DeterministicStrategy>();
    player2->setStrategy(std::move(strategy2));

    m_hand->initialize();
    m_hand->dealHoleCards(0);

    auto* preflop = dynamic_cast<pkt::core::PreflopState*>(&m_hand->getState());
    ASSERT_NE(preflop, nullptr);
    preflop->promptPlayerAction(*m_hand, *player1); // Folds

    // Player 1: 1 preflop hand, 0 flop/turn/river hands
    auto stats1 = store.loadPlayerStatistics(player1->getStrategyName());
    EXPECT_EQ(stats1[nbPlayers].preflopStatistics.hands, 1);
    EXPECT_EQ(stats1[nbPlayers].flopStatistics.hands, 0);
    EXPECT_EQ(stats1[nbPlayers].turnStatistics.hands, 0);
    EXPECT_EQ(stats1[nbPlayers].riverStatistics.hands, 0);

    // Player 2: 1 preflop hand, 0 flop hands (hand ended preflop)
    auto stats2 = store.loadPlayerStatistics(player2->getStrategyName());
    EXPECT_EQ(stats2[nbPlayers].preflopStatistics.hands, 1);
    EXPECT_EQ(stats2[nbPlayers].flopStatistics.hands, 0); // Hand ended preflop, no flop
}

// Test that multiple players with the same strategy combine stats correctly
TEST_F(SqlitePlayersStatisticsStoreTest, MultiplePlayersWithSameStrategyCombineCorrectly)
{
    auto& store = *m_playersStatisticsStore;
    int nbPlayers = 3;

    setupHandWithoutInitialize(nbPlayers, gameData);
    auto player1 = getPlayerById(m_actingPlayersList, 0);
    auto player2 = getPlayerById(m_actingPlayersList, 1);
    auto player3 = getPlayerById(m_actingPlayersList, 2);

    // All three players use the same strategy
    auto strategy1 = std::make_unique<pkt::test::DeterministicStrategy>();
    strategy1->setLastAction(pkt::core::GameState::Preflop, {player1->getId(), pkt::core::ActionType::Fold});
    player1->setStrategy(std::move(strategy1));

    auto strategy2 = std::make_unique<pkt::test::DeterministicStrategy>();
    strategy2->setLastAction(pkt::core::GameState::Preflop, {player2->getId(), pkt::core::ActionType::Fold});
    player2->setStrategy(std::move(strategy2));

    m_hand->initialize();
    m_hand->dealHoleCards(0);

    auto strategy3 = std::make_unique<pkt::test::DeterministicStrategy>();
    player3->setStrategy(std::move(strategy3));

    auto* preflop = dynamic_cast<pkt::core::PreflopState*>(&m_hand->getState());
    ASSERT_NE(preflop, nullptr);

    preflop->promptPlayerAction(*m_hand, *player1); // Player 1 folds
    preflop->promptPlayerAction(*m_hand, *player2); // Player 2 folds

    // All players share same strategy, so their stats combine
    // Player 1 + Player 2 both acted = 2 hands, 2 folds
    auto sharedStats = store.loadPlayerStatistics("DeterministicStrategy");
    EXPECT_EQ(sharedStats[nbPlayers].preflopStatistics.hands, 2);
    EXPECT_EQ(sharedStats[nbPlayers].preflopStatistics.folds, 2);
}

// Test baseline reset after load
TEST_F(SqlitePlayersStatisticsStoreTest, BaselineResetAfterLoad)
{
    auto& store = *m_playersStatisticsStore;
    int nbPlayers = 2;

    // First hand
    setupHandWithoutInitialize(nbPlayers, gameData);
    auto player1 = getPlayerById(m_actingPlayersList, 0);
    auto player2 = getPlayerById(m_actingPlayersList, 1);

    auto strategy1 = std::make_unique<pkt::test::DeterministicStrategy>();
    strategy1->setLastAction(pkt::core::GameState::Preflop, {player1->getId(), pkt::core::ActionType::Fold});
    player1->setStrategy(std::move(strategy1));

    auto strategy2 = std::make_unique<pkt::test::DeterministicStrategy>();
    player2->setStrategy(std::move(strategy2));

    m_hand->initialize();
    m_hand->dealHoleCards(0);

    auto* preflop = dynamic_cast<pkt::core::PreflopState*>(&m_hand->getState());
    ASSERT_NE(preflop, nullptr);
    preflop->promptPlayerAction(*m_hand, *player1);

    // After first hand: 1 hand, 1 fold
    auto statsAfterFirst = store.loadPlayerStatistics(player1->getStrategyName());
    EXPECT_EQ(statsAfterFirst[nbPlayers].preflopStatistics.hands, 1);
    EXPECT_EQ(statsAfterFirst[nbPlayers].preflopStatistics.folds, 1);

    // Simulate creating a new player that loads existing stats
    // This tests that the baseline is properly set after loading from DB
    setupHandWithoutInitialize(nbPlayers, gameData);
    player1 = getPlayerById(m_actingPlayersList, 0);
    player2 = getPlayerById(m_actingPlayersList, 1);

    strategy1 = std::make_unique<pkt::test::DeterministicStrategy>();
    strategy1->setLastAction(pkt::core::GameState::Preflop, {player1->getId(), pkt::core::ActionType::Fold});
    player1->setStrategy(std::move(strategy1));

    strategy2 = std::make_unique<pkt::test::DeterministicStrategy>();
    player2->setStrategy(std::move(strategy2));

    m_hand->initialize();
    m_hand->dealHoleCards(0);

    preflop = dynamic_cast<pkt::core::PreflopState*>(&m_hand->getState());
    ASSERT_NE(preflop, nullptr);
    preflop->promptPlayerAction(*m_hand, *player1);

    // After second hand: should be 2 hands, 2 folds (not 3 hands from bad delta calculation)
    auto statsAfterSecond = store.loadPlayerStatistics(player1->getStrategyName());
    EXPECT_EQ(statsAfterSecond[nbPlayers].preflopStatistics.hands, 2);
    EXPECT_EQ(statsAfterSecond[nbPlayers].preflopStatistics.folds, 2);
}

// Regression test: Verify no triangular number growth pattern
TEST_F(SqlitePlayersStatisticsStoreTest, NoTriangularNumberGrowth)
{
    auto& store = *m_playersStatisticsStore;
    int nbPlayers = 2;

    // Play 5 hands and verify linear growth (not 1, 3, 6, 10, 15...)
    for (int handNum = 1; handNum <= 5; ++handNum)
    {
        setupHandWithoutInitialize(nbPlayers, gameData);
        auto player1 = getPlayerById(m_actingPlayersList, 0);
        auto player2 = getPlayerById(m_actingPlayersList, 1);

        auto strategy1 = std::make_unique<pkt::test::DeterministicStrategy>();
        strategy1->setLastAction(pkt::core::GameState::Preflop, {player1->getId(), pkt::core::ActionType::Fold});
        player1->setStrategy(std::move(strategy1));

        auto strategy2 = std::make_unique<pkt::test::DeterministicStrategy>();
        player2->setStrategy(std::move(strategy2));

        m_hand->initialize();
        m_hand->dealHoleCards(0);

        auto* preflop = dynamic_cast<pkt::core::PreflopState*>(&m_hand->getState());
        ASSERT_NE(preflop, nullptr);
        preflop->promptPlayerAction(*m_hand, *player1);

        auto stats = store.loadPlayerStatistics(player1->getStrategyName());
        // Should be linear: handNum hands, handNum folds
        EXPECT_EQ(stats[nbPlayers].preflopStatistics.hands, handNum)
            << "After hand " << handNum << ", expected " << handNum << " hands (linear growth)";
        EXPECT_EQ(stats[nbPlayers].preflopStatistics.folds, handNum)
            << "After hand " << handNum << ", expected " << handNum << " folds (linear growth)";

        // Verify NOT triangular: 1, 3, 6, 10, 15...
        int triangular = handNum * (handNum + 1) / 2;
        if (handNum > 1)
        {
            EXPECT_NE(stats[nbPlayers].preflopStatistics.hands, triangular)
                << "After hand " << handNum << ", should NOT have triangular growth (" << triangular << ")";
        }
    }
}

} // namespace pkt::test
