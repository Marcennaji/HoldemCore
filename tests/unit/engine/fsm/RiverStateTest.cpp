// tests/RiverStateTest.cpp

#include "RiverStateTest.h"
#include <iostream>
#include "core/engine/model/PlayerAction.h"
#include "core/engine/state/RiverState.h"
#include "core/player/Helpers.h"

using namespace pkt::core;
using namespace pkt::core::player;
using namespace std;

namespace pkt::test
{

void RiverStateTest::logTestMessage(const std::string& message) const
{
    getLogger().info("RiverState : " + message);
}

void RiverStateTest::SetUp()
{
    EngineTest::SetUp();
    m_events.clear();
    m_events.onBettingRoundStarted = [&](GameState state) { m_lastGameState = state; };
}

void RiverStateTest::TearDown()
{
    EngineTest::TearDown();
}

TEST_F(RiverStateTest, StartRiverInitializesPlayersCorrectly)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Progress through preflop to flop
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Flop);

    // Progress through flop to turn
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Turn);

    // Progress through turn to river
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, River);

    // Verify players are still active and have valid cash amounts
    EXPECT_EQ(isPlayerStillActive(playerDealer->getId()), true);
    EXPECT_EQ(isPlayerStillActive(playerBb->getId()), true);
    EXPECT_GE(playerDealer->getCash(), 0);
    EXPECT_GE(playerBb->getCash(), 0);
}

TEST_F(RiverStateTest, OnlyChecksOnRiverShouldGoToPostRiver)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Progress through preflop, flop, and turn to reach river
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, River);

    // River actions - all checks should progress to PostRiver
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, PostRiver);
}

TEST_F(RiverStateTest, BetCallScenarioOnRiver)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Progress through preflop, flop, and turn to reach river
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, River);

    // River actions - bet and call
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Call});

    EXPECT_EQ(m_lastGameState, PostRiver);
}

TEST_F(RiverStateTest, AllInFoldScenarioOnRiver)
{
    initializeHandWithPlayers(2, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);

    // Progress through preflop, flop, and turn to reach river
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, River);

    // River action - all-in and fold
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Allin});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Fold});

    EXPECT_EQ(m_lastGameState, PostRiver);
}

TEST_F(RiverStateTest, RaiseOnRiverKeepsRoundOpen)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Progress through preflop, flop, and turn to reach river
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, River);

    // River actions - bet, raise, call
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Raise, 200});
    EXPECT_EQ(m_lastGameState, River);
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    EXPECT_EQ(m_lastGameState, PostRiver);
}

TEST_F(RiverStateTest, FoldOnRiverWithTwoPlayersEndsHand)
{
    initializeHandWithPlayers(2, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);

    // Progress through preflop, flop, and turn to reach river
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, River);

    // River action - bet and fold (only one player left)
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Bet, 100});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Fold});

    EXPECT_EQ(m_lastGameState, PostRiver);
}

TEST_F(RiverStateTest, MultipleFoldsOnRiver)
{
    initializeHandWithPlayers(4, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);
    auto playerUtg = getPlayerById(m_actingPlayersList, 3);

    // Progress through preflop, flop, and turn to reach river
    m_hand->handlePlayerAction({playerUtg->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    // Flop - all check
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerUtg->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Check});

    // Turn - all check
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerUtg->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, River);

    // River actions - bet, fold, fold, call
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerUtg->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Call});

    EXPECT_EQ(m_lastGameState, PostRiver);
}

TEST_F(RiverStateTest, CheckRaiseScenarioOnRiver)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Progress through preflop, flop, and turn to reach river
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, River);

    // River actions - check, bet, raise, call
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Bet, 100});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Raise, 200});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Call});

    EXPECT_EQ(m_lastGameState, PostRiver);
}

TEST_F(RiverStateTest, AllInCallScenarioOnRiver)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Progress through preflop, flop, and turn to reach river
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, River);

    // River actions - all-in, call
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Allin});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Call});

    EXPECT_EQ(m_lastGameState, PostRiver);
}

TEST_F(RiverStateTest, BetFoldScenarioOnRiver)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Progress through preflop, flop, and turn to reach river
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, River);

    // River actions - bet, fold
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 150});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Fold});

    EXPECT_EQ(m_lastGameState, PostRiver);
}

TEST_F(RiverStateTest, LargeBetOnRiverRequiresResponse)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Progress through preflop, flop, and turn to reach river
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, River);

    // River actions - large bet and call
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 500});
    EXPECT_EQ(m_lastGameState, River);
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Call});
    EXPECT_EQ(m_lastGameState, PostRiver);
}

TEST_F(RiverStateTest, MultipleRaisesOnRiver)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Progress through preflop, flop, and turn to reach river
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, River);

    // River actions - bet, raise, reraise, call
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Raise, 200});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Raise, 400});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Call});

    EXPECT_EQ(m_lastGameState, PostRiver);
}

TEST_F(RiverStateTest, HeadsUpRiverAction)
{
    initializeHandWithPlayers(2, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);

    // Progress through preflop, flop, and turn to reach river
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, River);

    // River actions - bet, raise, call
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Bet, 200});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Raise, 300});
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Call});

    EXPECT_EQ(m_lastGameState, PostRiver);
}

// Test for call-raise sequence mentioned in conversation
TEST_F(RiverStateTest, CallRaiseSequenceOnRiver)
{
    initializeHandWithPlayers(4, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);
    auto playerUtg = getPlayerById(m_actingPlayersList, 3);

    // Preflop: call, raise, call, call, call pattern
    m_hand->handlePlayerAction({playerUtg->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Raise, 50});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerUtg->getId(), ActionType::Call});

    // Flop - all check
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerUtg->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Check});

    // Turn - all check
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerUtg->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, River);

    // River actions - check, bet, call, call, call
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Bet, 100});
    m_hand->handlePlayerAction({playerUtg->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});

    EXPECT_EQ(m_lastGameState, PostRiver);
}

} // namespace pkt::test