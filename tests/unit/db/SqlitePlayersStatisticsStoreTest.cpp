#include "SqlitePlayersStatisticsStoreTest.h"
#include "common/DeterministicStrategy.h"
#include "common/EngineTest.h"
#include "common/FakeRandomizer.h"
#include "core/engine/EngineDefs.h"
#include "core/engine/state/PreflopState.h"
#include "core/interfaces/persistence/NullPlayersStatisticsStore.h"
#include "core/player/Helpers.h"
#include "core/services/PokerServices.h"
#include "core/services/ServiceContainer.h"
#include "infra/ConsoleLogger.h"
#include "infra/eval/PsimHandEvaluationEngine.h"
#include "infra/persistence/SqliteDb.h"

#include <memory>

using namespace pkt::core;
using namespace pkt::core::player;
using namespace pkt::infra;

using namespace std;
namespace pkt::test
{

void SqlitePlayersStatisticsStoreTest::SetUp()
{
    // First do the base setup to initialize game data and other essentials
    gameData.maxNumberOfPlayers = MAX_NUMBER_OF_PLAYERS;
    gameData.startMoney = 1000;
    gameData.firstSmallBlind = 10;
    gameData.tableProfile = TableProfile::RandomOpponents;

    // Create a ServiceContainer with the test database
    auto db = std::make_unique<pkt::infra::SqliteDb>(":memory:");
    auto store = std::make_unique<pkt::infra::SqlitePlayersStatisticsStore>(std::move(db));

    myTestServices = std::make_shared<pkt::core::AppServiceContainer>();
    myTestServices->setPlayersStatisticsStore(std::move(store));

    // Set up other services similar to EngineTest but in our ServiceContainer
    auto logger = std::make_unique<pkt::infra::ConsoleLogger>();
    logger->setLogLevel(pkt::core::LogLevel::Info);
    myTestServices->setLogger(std::move(logger));
    myTestServices->setHandEvaluationEngine(std::make_unique<pkt::infra::PsimHandEvaluationEngine>(myTestServices));
    auto randomizer = std::make_unique<FakeRandomizer>();
    randomizer->values = {3, 5, 7};
    myTestServices->setRandomizer(std::move(randomizer));

    // Create the factory with our test services
    auto pokerServices = std::make_shared<pkt::core::PokerServices>(myTestServices);
    myFactory = std::make_unique<EngineFactory>(myEvents, pokerServices);
}

void SqlitePlayersStatisticsStoreTest::TearDown()
{
    myTestServices.reset();
    EngineTest::TearDown();
}

TEST_F(SqlitePlayersStatisticsStoreTest, SaveAndLoadStatistics)
{
    auto& store = myTestServices->playersStatisticsStore();
    int nbPlayers = 3;
    initializeHandWithPlayers(nbPlayers, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

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
    auto* preflop = dynamic_cast<pkt::core::PreflopState*>(&myHand->getState());
    ASSERT_NE(preflop, nullptr);

    preflop->promptPlayerAction(*myHand, *playerDealer); // Dealer folds
    preflop->promptPlayerAction(*myHand, *playerSb);     // Small blind folds
    // -> round ends automatically, stats should be saved

    // Verify loaded statistics for the number of players (index 1-based)
    auto dealerStats = store.loadPlayerStatistics(playerDealer->getName());
    EXPECT_EQ(dealerStats[nbPlayers].preflopStatistics.hands, 1);
    EXPECT_EQ(dealerStats[nbPlayers].preflopStatistics.folds, 1);

    auto sbStats = store.loadPlayerStatistics(playerSb->getName());
    EXPECT_EQ(sbStats[nbPlayers].preflopStatistics.hands, 1);
    EXPECT_EQ(sbStats[nbPlayers].preflopStatistics.folds, 1);

    auto bbStats = store.loadPlayerStatistics(playerBb->getName());
    EXPECT_EQ(bbStats[nbPlayers].preflopStatistics.hands, 0); // Big blind did not act
}

} // namespace pkt::test
