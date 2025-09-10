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
    initializeHandFsmWithPlayers(3, gameData);

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

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

TEST_F(FlopStateTest, OnlyChecksFlopShouldGoToTurn)
{
    initializeHandFsmWithPlayers(3, gameData);

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

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

TEST_F(FlopStateTest, BetCallScenarioOnFlop)
{
    initializeHandFsmWithPlayers(3, gameData);

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Simulate preflop ending
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // Flop bet scenario
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 50});
    EXPECT_EQ(myLastGameState, Flop); // round still open

    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, Turn); // round closes after call
}

TEST_F(FlopStateTest, AllInOnFlop)
{
    initializeHandFsmWithPlayers(2, gameData);

    auto playerDealerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Preflop round finishing
    myHandFsm->handlePlayerAction({playerDealerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // Player goes all-in, so he is not any more "running" (--> we won't find it in acting players list)
    myHandFsm->handlePlayerAction({playerDealerSb->getId(), ActionType::Allin});
    EXPECT_EQ(getPlayerFsmById(mySeatsListFsm, playerDealerSb->getId())->getCash(), 0);
}

TEST_F(FlopStateTest, RaiseOnFlopKeepsRoundOpen)
{
    initializeHandFsmWithPlayers(3, gameData);

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Simulate preflop ending
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // Flop raise scenario
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 50});
    EXPECT_EQ(myLastGameState, Flop); // round still open

    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Raise, 100});
    EXPECT_EQ(myLastGameState, Flop); // round still open after raise

    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, Turn); // round closes after call
}

TEST_F(FlopStateTest, FoldOnFlopWithTwoPlayersEndsHand)
{
    initializeHandFsmWithPlayers(2, gameData);

    auto playerDealerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Preflop round finishing
    myHandFsm->handlePlayerAction({playerDealerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // One player bets, other folds - should end hand
    myHandFsm->handlePlayerAction({playerDealerSb->getId(), ActionType::Bet, 50});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Fold});

    // Hand should end with only one player remaining
    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(FlopStateTest, MultipleFoldsOnFlop)
{
    initializeHandFsmWithPlayers(4, gameData);

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);
    auto playerUtg = getPlayerFsmById(myActingPlayersListFsm, 3);

    // Simulate preflop ending with all players calling
    myHandFsm->handlePlayerAction({playerUtg->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);
    EXPECT_EQ(myActingPlayersListFsm->size(), 4);

    // Multiple folds on flop
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 50});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Fold});
    EXPECT_EQ(myActingPlayersListFsm->size(), 3);

    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    EXPECT_EQ(myActingPlayersListFsm->size(), 2);

    myHandFsm->handlePlayerAction({playerUtg->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, Turn); // Round should advance to Turn
}

TEST_F(FlopStateTest, CheckRaiseScenarioOnFlop)
{
    initializeHandFsmWithPlayers(3, gameData);

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Simulate preflop ending
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // Check-raise scenario
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Bet, 75});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Raise, 150});

    EXPECT_EQ(myLastGameState, Flop); // round still open after raise

    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, Turn); // round closes after call
}

TEST_F(FlopStateTest, AllInCallScenarioOnFlop)
{
    initializeHandFsmWithPlayers(3, gameData);

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Simulate preflop ending
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // One player goes all-in, other calls
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Allin});
    EXPECT_EQ(getPlayerFsmById(mySeatsListFsm, playerSb->getId())->getCash(), 0);

    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call});

    // Both players should be committed, round should advance to PostRiver
    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(FlopStateTest, BetFoldScenarioOnFlop)
{
    initializeHandFsmWithPlayers(3, gameData);

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Simulate preflop ending
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);
    size_t initialActingPlayers = myActingPlayersListFsm->size();

    // Bet and fold scenario
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Fold});

    // One player should have folded
    EXPECT_EQ(myActingPlayersListFsm->size(), initialActingPlayers - 1);
}

TEST_F(FlopStateTest, LargeBetOnFlopRequiresResponse)
{
    initializeHandFsmWithPlayers(3, gameData);

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Simulate preflop ending
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);

    // Large bet on flop
    int initialCash = playerSb->getCash();
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 200});

    // Verify bet was applied
    EXPECT_LT(playerSb->getCash(), initialCash);
    EXPECT_EQ(myLastGameState, Flop); // round still open, awaiting response
}

} // namespace pkt::test
