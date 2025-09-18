// tests/RiverStateTest.cpp

#include "RiverStateTest.h"
#include "core/engine/model/PlayerAction.h"
#include "core/engine/state/RiverState.h"
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
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Progress through preflop to flop
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // Progress through flop to turn
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);

    // Progress through turn to river
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // Verify players are still active and have valid cash amounts
    EXPECT_EQ(isPlayerStillActive(playerDealer->getId()), true);
    EXPECT_EQ(isPlayerStillActive(playerBb->getId()), true);
    EXPECT_GE(playerDealer->getCash(), 0);
    EXPECT_GE(playerBb->getCash(), 0);
}

TEST_F(RiverStateTest, OnlyChecksOnRiverShouldGoToPostRiver)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Progress through preflop, flop, and turn to reach river
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - all checks should progress to PostRiver
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, BetCallScenarioOnRiver)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Progress through preflop, flop, and turn to reach river
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - bet and call
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Call});

    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, AllInFoldScenarioOnRiver)
{
    initializeHandWithPlayers(2, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);

    // Progress through preflop, flop, and turn to reach river
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River action - all-in and fold
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Allin});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Fold});

    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, RaiseOnRiverKeepsRoundOpen)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Progress through preflop, flop, and turn to reach river
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - bet, raise, call
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Raise, 200});
    EXPECT_EQ(myLastGameState, River);
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, FoldOnRiverWithTwoPlayersEndsHand)
{
    initializeHandWithPlayers(2, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);

    // Progress through preflop, flop, and turn to reach river
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River action - bet and fold (only one player left)
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Bet, 100});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Fold});

    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, MultipleFoldsOnRiver)
{
    initializeHandWithPlayers(4, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);
    auto playerUtg = getPlayerById(myActingPlayersList, 3);

    // Progress through preflop, flop, and turn to reach river
    myHand->handlePlayerAction({playerUtg->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    // Flop - all check
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerUtg->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Check});

    // Turn - all check
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerUtg->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - bet, fold, fold, call
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerUtg->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Call});

    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, CheckRaiseScenarioOnRiver)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Progress through preflop, flop, and turn to reach river
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - check, bet, raise, call
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Bet, 100});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Raise, 200});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Call});

    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, AllInCallScenarioOnRiver)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Progress through preflop, flop, and turn to reach river
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - all-in, call
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Allin});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Call});

    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, BetFoldScenarioOnRiver)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Progress through preflop, flop, and turn to reach river
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - bet, fold
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 150});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Fold});

    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, LargeBetOnRiverRequiresResponse)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Progress through preflop, flop, and turn to reach river
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - large bet and call
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 500});
    EXPECT_EQ(myLastGameState, River);
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, MultipleRaisesOnRiver)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Progress through preflop, flop, and turn to reach river
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - bet, raise, reraise, call
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Raise, 200});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Raise, 400});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Call});

    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(RiverStateTest, HeadsUpRiverAction)
{
    initializeHandWithPlayers(2, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);

    // Progress through preflop, flop, and turn to reach river
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - bet, raise, call
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Bet, 200});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Raise, 300});
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Call});

    EXPECT_EQ(myLastGameState, PostRiver);
}

// Test for call-raise sequence mentioned in conversation
TEST_F(RiverStateTest, CallRaiseSequenceOnRiver)
{
    initializeHandWithPlayers(4, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);
    auto playerUtg = getPlayerById(myActingPlayersList, 3);

    // Preflop: call, raise, call, call, call pattern
    myHand->handlePlayerAction({playerUtg->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Raise, 50});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerUtg->getId(), ActionType::Call});

    // Flop - all check
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerUtg->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Check});

    // Turn - all check
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerUtg->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);

    // River actions - check, bet, call, call, call
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Bet, 100});
    myHand->handlePlayerAction({playerUtg->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});

    EXPECT_EQ(myLastGameState, PostRiver);
}

} // namespace pkt::test