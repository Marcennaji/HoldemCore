// tests/FlopState.cpp

#include "FlopStateTest.h"
#include <iostream>
#include "core/engine/model/PlayerAction.h"
#include "core/engine/state/FlopState.h"
#include "core/player/Helpers.h"

using namespace pkt::core;
using namespace pkt::core::player;
using namespace std;

namespace pkt::test
{

void FlopStateTest::logTestMessage(const std::string& message) const
{
    getLogger().info("FlopState : " + message);
}

void FlopStateTest::SetUp()
{
    EngineTest::SetUp();
    m_events.clear();
    m_events.onBettingRoundStarted = [&](GameState state) { m_lastGameState = state; };
}

void FlopStateTest::TearDown()
{
    EngineTest::TearDown();
}

TEST_F(FlopStateTest, StartFlopInitializesPlayersCorrectly)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Force preflop actions to move FSM to Flop
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Flop);

    // Verify initial cash, bet amounts, or any Flop-specific context
    EXPECT_GE(playerDealer->getCash(), 0);
    EXPECT_GE(playerSb->getCash(), 0);
    EXPECT_GE(playerBb->getCash(), 0);
}

TEST_F(FlopStateTest, OnlyChecksFlopShouldGoToTurn)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Simulate preflop ending
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Flop);

    // Flop actions
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Turn);
}

TEST_F(FlopStateTest, BetCallScenarioOnFlop)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Simulate preflop ending
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Flop);

    // Flop bet scenario
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 50});
    EXPECT_EQ(m_lastGameState, Flop); // round still open

    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Call});
    EXPECT_EQ(m_lastGameState, Turn); // round closes after call
}

TEST_F(FlopStateTest, AllInOnFlop)
{
    initializeHandWithPlayers(2, gameData);

    auto playerDealerSb = getPlayerById(m_actingPlayersList, 0);
    auto playerBb = getPlayerById(m_actingPlayersList, 1);

    // Preflop round finishing
    m_hand->handlePlayerAction({playerDealerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Flop);

    // Player goes all-in, so he is not any more "running" (--> we won't find it in acting players list)
    m_hand->handlePlayerAction({playerDealerSb->getId(), ActionType::Allin});
    EXPECT_EQ(getPlayerById(m_seatsList, playerDealerSb->getId())->getCash(), 0);
}

TEST_F(FlopStateTest, RaiseOnFlopKeepsRoundOpen)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Simulate preflop ending
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Flop);

    // Flop raise scenario
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 50});
    EXPECT_EQ(m_lastGameState, Flop); // round still open

    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Raise, 100});
    EXPECT_EQ(m_lastGameState, Flop); // round still open after raise

    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    EXPECT_EQ(m_lastGameState, Turn); // round closes after call
}

TEST_F(FlopStateTest, FoldOnFlopWithTwoPlayersEndsHand)
{
    initializeHandWithPlayers(2, gameData);

    auto playerDealerSb = getPlayerById(m_actingPlayersList, 0);
    auto playerBb = getPlayerById(m_actingPlayersList, 1);

    // Preflop round finishing
    m_hand->handlePlayerAction({playerDealerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Flop);

    // One player bets, other folds - should end hand
    m_hand->handlePlayerAction({playerDealerSb->getId(), ActionType::Bet, 50});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Fold});

    // Hand should end with only one player remaining
    EXPECT_EQ(m_lastGameState, PostRiver);
}

TEST_F(FlopStateTest, MultipleFoldsOnFlop)
{
    initializeHandWithPlayers(4, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);
    auto playerUtg = getPlayerById(m_actingPlayersList, 3);

    // Simulate preflop ending with all players calling
    m_hand->handlePlayerAction({playerUtg->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Flop);
    EXPECT_EQ(m_actingPlayersList->size(), 4);

    // Multiple folds on flop
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 50});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Fold});
    EXPECT_EQ(m_actingPlayersList->size(), 3);

    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    EXPECT_EQ(m_actingPlayersList->size(), 2);

    m_hand->handlePlayerAction({playerUtg->getId(), ActionType::Call});
    EXPECT_EQ(m_lastGameState, Turn); // Round should advance to Turn
}

TEST_F(FlopStateTest, CheckRaiseScenarioOnFlop)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Simulate preflop ending
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Flop);

    // Check-raise scenario
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Bet, 75});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Raise, 150});

    EXPECT_EQ(m_lastGameState, Flop); // round still open after raise

    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Call});
    EXPECT_EQ(m_lastGameState, Turn); // round closes after call
}

TEST_F(FlopStateTest, AllInCallScenarioOnFlop)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Simulate preflop ending
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Flop);

    // One player goes all-in, other calls
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Allin});
    EXPECT_EQ(getPlayerById(m_seatsList, playerSb->getId())->getCash(), 0);

    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Call});

    // Both players are all-in, but should advance to Turn first (not skip directly to PostRiver)
    EXPECT_EQ(m_lastGameState, Turn);
}

TEST_F(FlopStateTest, BetFoldScenarioOnFlop)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Simulate preflop ending
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Flop);
    size_t initialActingPlayers = m_actingPlayersList->size();

    // Bet and fold scenario
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Fold});

    // One player should have folded
    EXPECT_EQ(m_actingPlayersList->size(), initialActingPlayers - 1);
}

TEST_F(FlopStateTest, LargeBetOnFlopRequiresResponse)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Simulate preflop ending
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Flop);

    // Large bet on flop
    int initialCash = playerSb->getCash();
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 200});

    // Verify bet was applied
    EXPECT_LT(playerSb->getCash(), initialCash);
    EXPECT_EQ(m_lastGameState, Flop); // round still open, awaiting response
}

} // namespace pkt::test
