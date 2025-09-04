#include "SqlitePlayersStatisticsStoreTest.h"
#include "common/EngineTest.h"
#include "core/interfaces/persistence/NullPlayersStatisticsStore.h"
#include "core/player/Helpers.h"
#include "core/services/GlobalServices.h"
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
    EngineTest::SetUp();
    auto db = std::make_unique<pkt::infra::SqliteDb>(":memory:");
    auto& services = pkt::core::GlobalServices::instance();
    auto store = std::make_unique<pkt::infra::SqlitePlayersStatisticsStore>(std::move(db));
    services.setPlayersStatisticsStore(std::move(store));
}

void SqlitePlayersStatisticsStoreTest::TearDown()
{

    auto& services = pkt::core::GlobalServices::instance();
    services.setPlayersStatisticsStore(std::make_unique<NullPlayersStatisticsStore>());
}

TEST_F(SqlitePlayersStatisticsStoreTest, SaveAndLoadSinglePlayer)
{
    auto& store = pkt::core::GlobalServices::instance().playersStatisticsStore();
    int nbPlayers = 3;
    initializeHandFsmForTesting(nbPlayers, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myRunningPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myRunningPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myRunningPlayersListFsm, 2);

    playerDealer->updateCurrentHandContext(GameState::Preflop, *myHandFsm);
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});

    playerSb->updateCurrentHandContext(GameState::Preflop, *myHandFsm);
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Fold});
    // -> then, the round ends, and the players stats should be automatically saved in the database

    // Verify loaded statistics for the number of players (index 1-based)
    auto dealerStats = store.loadPlayerStatistics(playerDealer->getName());
    EXPECT_EQ(dealerStats[nbPlayers].preflopStatistics.hands, 1);
    EXPECT_EQ(dealerStats[nbPlayers].preflopStatistics.folds, 1);

    auto sbStats = store.loadPlayerStatistics(playerSb->getName());
    EXPECT_EQ(sbStats[nbPlayers].preflopStatistics.hands, 1);
    EXPECT_EQ(sbStats[nbPlayers].preflopStatistics.folds, 1);

    auto bbStats = store.loadPlayerStatistics(playerBb->getName());
    EXPECT_EQ(bbStats[nbPlayers].preflopStatistics.hands, 0);
}

} // namespace pkt::test
