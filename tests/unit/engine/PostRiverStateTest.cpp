#include "PostRiverStateTest.h"
#include "core/engine/EngineDefs.h"
#include "core/engine/round_states/PostRiverState.h"
#include "core/engine/round_states/PreflopState.h"
#include "core/player/Helpers.h"
#include "core/services/GlobalServices.h"

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

void PostRiverStateTest::SetUp()
{
    EngineTest::SetUp();
    myEvents.clear();
    myEvents.onBettingRoundStarted = [&](GameState state) { myLastGameState = state; };
}

void PostRiverStateTest::TearDown()
{
    EngineTest::TearDown();
}

void PostRiverStateTest::logTestMessage(const std::string& message)
{
    GlobalServices::instance().logger().info("PostRiverStateTest: " + message);
}

TEST_F(PostRiverStateTest, SampleTest)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Progress through all states to reach post-river
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    EXPECT_EQ(myLastGameState, Flop);
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    EXPECT_EQ(myLastGameState, Turn);
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    EXPECT_EQ(myLastGameState, River);
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 150});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Fold});

    EXPECT_EQ(myLastGameState, PostRiver);
}
} // namespace pkt::test
