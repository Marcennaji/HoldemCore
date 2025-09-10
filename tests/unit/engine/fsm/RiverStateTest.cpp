// tests/RiverStateTest.cpp

#include "RiverStateTest.h"
#include "core/engine/model/PlayerAction.h"
#include "core/engine/round_states/RiverState.h"
#include "core/player/Helpers.h"
#include "core/services/GlobalServices.h"

using namespace pkt::core;
using namespace pkt::core::player;
using namespace std;

namespace pkt::test
{

void RiverStateTest::logTestMessage(const std::string& message) const
{
    GlobalServices::instance().logger().verbose("RiverState : " + message);
}

void RiverStateTest::SetUp()
{
    EngineTest::SetUp();
    myEvents.clear();
    myEvents.onBettingRoundStarted = [&](GameState state) { myLastGameState = state; };
}

void RiverStateTest::TearDown()
{
    EngineTest::TearDown();
}

TEST_F(RiverStateTest, StartRiverInitializesPlayersCorrectly)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->initialize();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Progress through preflop to flop
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // Progress through flop to turn
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);

    // Progress through turn to river
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // Verify players are still active and have valid cash amounts
    EXPECT_EQ(isPlayerStillActive(playerDealer->getId()), true);
    EXPECT_EQ(isPlayerStillActive(playerBb->getId()), true);
    EXPECT_GE(playerDealer->getCash(), 0);
    EXPECT_GE(playerBb->getCash(), 0);
}

TEST_F(RiverStateTest, OnlyChecksOnRiverShouldGoToPostRiver)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->initialize();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Progress through preflop, flop, and turn to reach river
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - all checks should progress to PostRiver
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, BetCallScenarioOnRiver)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->initialize();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Progress through preflop, flop, and turn to reach river
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - bet and call
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call});

    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, AllInFoldScenarioOnRiver)
{
    initializeHandFsmForTesting(2, gameData);
    myHandFsm->initialize();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Progress through preflop, flop, and turn to reach river
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River action - all-in and fold
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Allin});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Fold});

    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, RaiseOnRiverKeepsRoundOpen)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->initialize();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Progress through preflop, flop, and turn to reach river
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - bet, raise, call
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Raise, 200});
    EXPECT_EQ(myLastGameState, River);
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, FoldOnRiverWithTwoPlayersEndsHand)
{
    initializeHandFsmForTesting(2, gameData);
    myHandFsm->initialize();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Progress through preflop, flop, and turn to reach river
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River action - bet and fold (only one player left)
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Bet, 100});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Fold});

    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, MultipleFoldsOnRiver)
{
    initializeHandFsmForTesting(4, gameData);
    myHandFsm->initialize();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);
    auto playerUtg = getPlayerFsmById(myActingPlayersListFsm, 3);

    // Progress through preflop, flop, and turn to reach river
    myHandFsm->handlePlayerAction({playerUtg->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    // Flop - all check
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerUtg->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Check});

    // Turn - all check
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerUtg->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - bet, fold, fold, call
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerUtg->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Call});

    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, CheckRaiseScenarioOnRiver)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->initialize();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Progress through preflop, flop, and turn to reach river
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - check, bet, raise, call
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Bet, 100});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Raise, 200});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call});

    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, AllInCallScenarioOnRiver)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->initialize();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Progress through preflop, flop, and turn to reach river
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - all-in, call
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Allin});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call});

    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, BetFoldScenarioOnRiver)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->initialize();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Progress through preflop, flop, and turn to reach river
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - bet, fold
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 150});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Fold});

    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, LargeBetOnRiverRequiresResponse)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->initialize();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Progress through preflop, flop, and turn to reach river
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - large bet and call
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 500});
    EXPECT_EQ(myLastGameState, River);
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, MultipleRaisesOnRiver)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->initialize();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Progress through preflop, flop, and turn to reach river
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - bet, raise, reraise, call
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Raise, 200});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Raise, 400});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call});

    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, HeadsUpRiverAction)
{
    initializeHandFsmForTesting(2, gameData);
    myHandFsm->initialize();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Progress through preflop, flop, and turn to reach river
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - bet, raise, call
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Bet, 200});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Raise, 300});
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Call});

    EXPECT_EQ(myLastGameState, PostRiver);
}

// Test for call-raise sequence mentioned in conversation
TEST_F(RiverStateTest, CallRaiseSequenceOnRiver)
{
    initializeHandFsmForTesting(4, gameData);
    myHandFsm->initialize();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);
    auto playerUtg = getPlayerFsmById(myActingPlayersListFsm, 3);

    // Preflop: call, raise, call, call, call pattern
    myHandFsm->handlePlayerAction({playerUtg->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Raise, 50});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerUtg->getId(), ActionType::Call});

    // Flop - all check
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerUtg->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Check});

    // Turn - all check
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerUtg->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - check, bet, call, call, call
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Bet, 100});
    myHandFsm->handlePlayerAction({playerUtg->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});

    EXPECT_EQ(myLastGameState, PostRiver);
}

} // namespace pkt::test