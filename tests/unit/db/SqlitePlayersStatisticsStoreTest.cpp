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

TEST_F(SqlitePlayersStatisticsStoreTest, SaveAndLoadStatistics)
{
    auto& store = *m_playersStatisticsStore;
    int nbPlayers = 3;
    initializeHandWithPlayers(nbPlayers, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Inject deterministic strategies
    auto dealerStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    dealerStrategy->setLastAction(pkt::core::GameState::Preflop, {playerDealer->getId(), pkt::core::ActionType::Fold});
    playerDealer->setStrategy(std::move(dealerStrategy));

    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerSb->getId(), pkt::core::ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    // Big blind does nothing here; we don’t configure Preflop action
    playerBb->setStrategy(std::move(bbStrategy));

    // Simulate preflop actions via state::promptPlayerAction
    auto* preflop = dynamic_cast<pkt::core::PreflopState*>(&m_hand->getState());
    ASSERT_NE(preflop, nullptr);

    preflop->promptPlayerAction(*m_hand, *playerDealer); // Dealer folds
    preflop->promptPlayerAction(*m_hand, *playerSb);     // Small blind folds
    // -> round ends automatically, stats should be saved

    // Note: All three players use DeterministicStrategy, so they share the same statistics record
    // Verify loaded statistics for the strategy (not individual players)
    auto dealerStats = store.loadPlayerStatistics(playerDealer->getStrategyTypeName());
    // Only Dealer and SB acted (both folded), BB didn't get a chance to act
    // Total: 2 hands, 2 folds
    EXPECT_EQ(dealerStats[nbPlayers].preflopStatistics.hands, 2);
    EXPECT_EQ(dealerStats[nbPlayers].preflopStatistics.folds, 2);

    auto sbStats = store.loadPlayerStatistics(playerSb->getStrategyTypeName());
    // Same strategy, same stats
    EXPECT_EQ(sbStats[nbPlayers].preflopStatistics.hands, 2);
    EXPECT_EQ(sbStats[nbPlayers].preflopStatistics.folds, 2);

    auto bbStats = store.loadPlayerStatistics(playerBb->getStrategyTypeName());
    // Same strategy, same stats
    EXPECT_EQ(bbStats[nbPlayers].preflopStatistics.hands, 2);
    EXPECT_EQ(bbStats[nbPlayers].preflopStatistics.folds, 2);
}

} // namespace pkt::test
