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
    myEvents.clear();
    myEvents.onBettingRoundStarted = [&](GameState state) { myLastGameState = state; };
}

void FlopStateTest::TearDown()
{
    EngineTest::TearDown();
}

TEST_F(FlopStateTest, StartFlopInitializesPlayersCorrectly)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Force preflop actions to move FSM to Flop
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // Verify initial cash, bet amounts, or any Flop-specific context
    EXPECT_GE(playerDealer->getCash(), 0);
    EXPECT_GE(playerSb->getCash(), 0);
    EXPECT_GE(playerBb->getCash(), 0);
}

TEST_F(FlopStateTest, OnlyChecksFlopShouldGoToTurn)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Simulate preflop ending
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // Flop actions
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);
}

TEST_F(FlopStateTest, BetCallScenarioOnFlop)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Simulate preflop ending
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // Flop bet scenario
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 50});
    EXPECT_EQ(myLastGameState, Flop); // round still open

    myHand->handlePlayerAction({playerBb->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, Turn); // round closes after call
}

TEST_F(FlopStateTest, AllInOnFlop)
{
    initializeHandWithPlayers(2, gameData);

    auto playerDealerSb = getPlayerById(myActingPlayersList, 0);
    auto playerBb = getPlayerById(myActingPlayersList, 1);

    // Preflop round finishing
    myHand->handlePlayerAction({playerDealerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // Player goes all-in, so he is not any more "running" (--> we won't find it in acting players list)
    myHand->handlePlayerAction({playerDealerSb->getId(), ActionType::Allin});
    EXPECT_EQ(getPlayerById(mySeatsList, playerDealerSb->getId())->getCash(), 0);
}

TEST_F(FlopStateTest, RaiseOnFlopKeepsRoundOpen)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Simulate preflop ending
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // Flop raise scenario
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 50});
    EXPECT_EQ(myLastGameState, Flop); // round still open

    myHand->handlePlayerAction({playerBb->getId(), ActionType::Raise, 100});
    EXPECT_EQ(myLastGameState, Flop); // round still open after raise

    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, Turn); // round closes after call
}

TEST_F(FlopStateTest, FoldOnFlopWithTwoPlayersEndsHand)
{
    initializeHandWithPlayers(2, gameData);

    auto playerDealerSb = getPlayerById(myActingPlayersList, 0);
    auto playerBb = getPlayerById(myActingPlayersList, 1);

    // Preflop round finishing
    myHand->handlePlayerAction({playerDealerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // One player bets, other folds - should end hand
    myHand->handlePlayerAction({playerDealerSb->getId(), ActionType::Bet, 50});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Fold});

    // Hand should end with only one player remaining
    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(FlopStateTest, MultipleFoldsOnFlop)
{
    initializeHandWithPlayers(4, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);
    auto playerUtg = getPlayerById(myActingPlayersList, 3);

    // Simulate preflop ending with all players calling
    myHand->handlePlayerAction({playerUtg->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);
    EXPECT_EQ(myActingPlayersList->size(), 4);

    // Multiple folds on flop
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 50});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Fold});
    EXPECT_EQ(myActingPlayersList->size(), 3);

    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    EXPECT_EQ(myActingPlayersList->size(), 2);

    myHand->handlePlayerAction({playerUtg->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, Turn); // Round should advance to Turn
}

TEST_F(FlopStateTest, CheckRaiseScenarioOnFlop)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Simulate preflop ending
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // Check-raise scenario
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Bet, 75});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Raise, 150});

    EXPECT_EQ(myLastGameState, Flop); // round still open after raise

    myHand->handlePlayerAction({playerBb->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, Turn); // round closes after call
}

TEST_F(FlopStateTest, AllInCallScenarioOnFlop)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Simulate preflop ending
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // One player goes all-in, other calls
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Allin});
    EXPECT_EQ(getPlayerById(mySeatsList, playerSb->getId())->getCash(), 0);

    myHand->handlePlayerAction({playerBb->getId(), ActionType::Call});

    // Both players should be committed, round should advance to PostRiver
    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(FlopStateTest, BetFoldScenarioOnFlop)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Simulate preflop ending
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);
    size_t initialActingPlayers = myActingPlayersList->size();

    // Bet and fold scenario
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Fold});

    // One player should have folded
    EXPECT_EQ(myActingPlayersList->size(), initialActingPlayers - 1);
}

TEST_F(FlopStateTest, LargeBetOnFlopRequiresResponse)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Simulate preflop ending
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // Large bet on flop
    int initialCash = playerSb->getCash();
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 200});

    // Verify bet was applied
    EXPECT_LT(playerSb->getCash(), initialCash);
    EXPECT_EQ(myLastGameState, Flop); // round still open, awaiting response
}

} // namespace pkt::test
