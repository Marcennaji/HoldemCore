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
    auto db = std::make_shared<pkt::infra::SqliteDb>(":memory:");
    auto& services = pkt::core::GlobalServices::instance();
    store = std::make_shared<pkt::infra::SqlitePlayersStatisticsStore>(db);
    services.setPlayersStatisticsStore(store);
}

void SqlitePlayersStatisticsStoreTest::TearDown()
{

    auto& services = pkt::core::GlobalServices::instance();
    services.setPlayersStatisticsStore(std::make_shared<NullPlayersStatisticsStore>());
}

TEST_F(SqlitePlayersStatisticsStoreTest, SaveAndLoadSinglePlayer)
{
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
    // -> the round ends, the players stats are automatically saved in the database

    auto dealerStats = store->loadPlayerStatistics(playerDealer->getName());

    // Verify loaded statistics for the number of players (index 1-based)
    EXPECT_GT(dealerStats[nbPlayers].preflopStatistics.hands, 0);
    EXPECT_GT(dealerStats[nbPlayers].preflopStatistics.folds, 0);
}

} // namespace pkt::test
