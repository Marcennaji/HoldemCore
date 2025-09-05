// tests/FlopState.cpp

#include "FlopStateTest.h"
#include "core/engine/model/PlayerAction.h"
#include "core/engine/round_states/FlopState.h"
#include "core/player/Helpers.h"
#include "core/services/GlobalServices.h"

using namespace pkt::core;
using namespace pkt::core::player;
using namespace std;

namespace pkt::test
{

void FlopStateTest::logTestMessage(const std::string& message) const
{
    GlobalServices::instance().logger().verbose("FlopState : " + message);
}

void FlopStateTest::SetUp()
{
    EngineTest::SetUp();
    myEvents.clear();
    myEvents.onBettingRoundStarted = [&](GameState state) { myLastGameState = state; };
}

void FlopStateTest::TearDown()
{
    EngineTest::TearDown();
}

TEST_F(FlopStateTest, StartFlopInitializesPlayersCorrectly)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myRunningPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myRunningPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myRunningPlayersListFsm, 2);

    // Force preflop actions to move FSM to Flop
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // Verify initial cash, bet amounts, or any Flop-specific context
    EXPECT_GE(playerDealer->getCash(), 0);
    EXPECT_GE(playerSb->getCash(), 0);
    EXPECT_GE(playerBb->getCash(), 0);
}

TEST_F(FlopStateTest, DISABLED_OnlyChecksFlopShouldGoToTurn)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myRunningPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myRunningPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myRunningPlayersListFsm, 2);

    // Simulate preflop ending
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // Flop actions
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);
}

TEST_F(FlopStateTest, RaiseOnFlopKeepsRoundOpen)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myRunningPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myRunningPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myRunningPlayersListFsm, 2);

    // Simulate preflop ending
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // Flop raise scenario
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Raise, 50});
    EXPECT_EQ(myLastGameState, Flop); // round still open

    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call, 50});
    EXPECT_EQ(myLastGameState, Turn); // round closes after call
}

TEST_F(FlopStateTest, AllInOnFlop)
{
    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    auto playerDealerSb = getPlayerFsmById(myRunningPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myRunningPlayersListFsm, 1);

    // Preflop round finishing
    myHandFsm->handlePlayerAction({playerDealerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // Player goes all-in
    myHandFsm->handlePlayerAction({playerDealerSb->getId(), ActionType::Allin});
    EXPECT_EQ(getPlayerFsmById(myRunningPlayersListFsm, playerDealerSb->getId())->getCash(), 0);
}

} // namespace pkt::test
