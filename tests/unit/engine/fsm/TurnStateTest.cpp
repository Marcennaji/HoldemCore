// tests/TurnStateTest.cpp

#include "TurnStateTest.h"
#include <iostream>
#include "core/engine/model/PlayerAction.h"
#include "core/engine/state/TurnState.h"
#include "core/player/Helpers.h"

using namespace pkt::core;
using namespace pkt::core::player;
using namespace std;

namespace pkt::test
{

void TurnStateTest::logTestMessage(const std::string& message) const
{
    getLogger().info("TurnState : " + message);
}

void TurnStateTest::SetUp()
{
    EngineTest::SetUp();
    myEvents.clear();
    myEvents.onBettingRoundStarted = [&](GameState state) { myLastGameState = state; };
}

void TurnStateTest::TearDown()
{
    EngineTest::TearDown();
}

TEST_F(TurnStateTest, StartTurnInitializesPlayersCorrectly)
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

    // Verify players are still active and have valid cash amounts
    EXPECT_EQ(isPlayerStillActive(playerDealer->getId()), true);
    EXPECT_EQ(isPlayerStillActive(playerBb->getId()), true);
    EXPECT_GE(playerDealer->getCash(), 0);
    EXPECT_GE(playerBb->getCash(), 0);
}

TEST_F(TurnStateTest, OnlyChecksOnTurnShouldGoToRiver)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Progress through preflop and flop
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);

    // Turn actions - all checks should progress to River
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);
}

TEST_F(TurnStateTest, BetOnTurnKeepsRoundOpen)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Progress through preflop and flop
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);

    // Turn bet scenario
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    EXPECT_EQ(myLastGameState, Turn); // round still open

    myHand->handlePlayerAction({playerBb->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, River); // round closes after call
}

TEST_F(TurnStateTest, RaiseOnTurnKeepsRoundOpen)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Progress through preflop and flop
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);

    // Turn raise scenario
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 75});
    EXPECT_EQ(myLastGameState, Turn); // round still open

    myHand->handlePlayerAction({playerBb->getId(), ActionType::Raise, 150});
    EXPECT_EQ(myLastGameState, Turn); // round still open after raise

    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, River); // round closes after call
}

TEST_F(TurnStateTest, FoldOnTurnWithTwoPlayersEndsHand)
{
    initializeHandWithPlayers(2, gameData);

    auto playerDealerSb = getPlayerById(myActingPlayersList, 0);
    auto playerBb = getPlayerById(myActingPlayersList, 1);

    // Progress through preflop and flop
    myHand->handlePlayerAction({playerDealerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerDealerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);

    // One player bets, other folds - should end hand
    myHand->handlePlayerAction({playerDealerSb->getId(), ActionType::Bet, 80});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Fold});

    // Hand should end with only one player remaining
    EXPECT_EQ(myActingPlayersList->size(), 1);
    EXPECT_EQ(myLastGameState, PostRiver); // Hand ends, no further rounds
}

TEST_F(TurnStateTest, AllInOnTurn)
{
    initializeHandWithPlayers(2, gameData);

    auto playerDealerSb = getPlayerById(myActingPlayersList, 0);
    auto playerBb = getPlayerById(myActingPlayersList, 1);

    // Progress through preflop and flop
    myHand->handlePlayerAction({playerDealerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerDealerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);

    // Player goes all-in on turn
    myHand->handlePlayerAction({playerDealerSb->getId(), ActionType::Allin});
    EXPECT_EQ(getPlayerById(mySeatsList, playerDealerSb->getId())->getCash(), 0);
    EXPECT_EQ(myLastGameState, Turn); // Round still open after all-in
}

TEST_F(TurnStateTest, MultipleFoldsOnTurn)
{
    initializeHandWithPlayers(4, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);
    auto playerUtg = getPlayerById(myActingPlayersList, 3);

    // Progress through preflop and flop with all players
    myHand->handlePlayerAction({playerUtg->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    // Flop - all check
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerUtg->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);
    EXPECT_EQ(myActingPlayersList->size(), 4);

    // Multiple folds on turn
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 120});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Fold});
    EXPECT_EQ(myActingPlayersList->size(), 3);

    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    EXPECT_EQ(myActingPlayersList->size(), 2);

    myHand->handlePlayerAction({playerUtg->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, River); // Round should advance to River
}

TEST_F(TurnStateTest, CheckRaiseScenarioOnTurn)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Progress through preflop and flop
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);

    // Check-raise scenario on turn
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Bet, 100});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Raise, 200});

    EXPECT_EQ(myLastGameState, Turn); // round still open after raise

    myHand->handlePlayerAction({playerBb->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, River); // round closes after call
}

TEST_F(TurnStateTest, AllInCallScenarioOnTurn)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Progress through preflop and flop
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);

    // One player goes all-in, other calls
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Allin});
    EXPECT_EQ(getPlayerById(mySeatsList, playerSb->getId())->getCash(), 0);

    myHand->handlePlayerAction({playerBb->getId(), ActionType::Call});

    // Both players should be allin, round should advance
    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(TurnStateTest, BetFoldScenarioOnTurn)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Progress through preflop and flop
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);
    size_t initialActingPlayers = myActingPlayersList->size();

    // Bet and fold scenario
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 150});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Fold});

    // One player should have folded
    EXPECT_EQ(myActingPlayersList->size(), initialActingPlayers - 1);
    EXPECT_EQ(myLastGameState, PostRiver); // Hand ends, no further rounds
}

TEST_F(TurnStateTest, LargeBetOnTurnRequiresResponse)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Progress through preflop and flop
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);

    // Large bet on turn
    int initialCash = playerSb->getCash();
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 300});

    // Verify bet was applied
    EXPECT_LT(playerSb->getCash(), initialCash);
    EXPECT_EQ(myLastGameState, Turn); // round still open, awaiting response
}

TEST_F(TurnStateTest, MultipleRaisesOnTurn)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Progress through preflop and flop
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);

    // Multiple raises scenario
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 50});
    EXPECT_EQ(myLastGameState, Turn); // round still open

    myHand->handlePlayerAction({playerBb->getId(), ActionType::Raise, 100});
    EXPECT_EQ(myLastGameState, Turn); // round still open after first raise

    myHand->handlePlayerAction({playerSb->getId(), ActionType::Raise, 200});
    EXPECT_EQ(myLastGameState, Turn); // round still open after second raise

    myHand->handlePlayerAction({playerBb->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, River); // round closes after call
}

TEST_F(TurnStateTest, HeadsUpTurnAction)
{
    initializeHandWithPlayers(2, gameData);

    auto playerDealerSb = getPlayerById(myActingPlayersList, 0);
    auto playerBb = getPlayerById(myActingPlayersList, 1);

    // Progress through preflop and flop
    myHand->handlePlayerAction({playerDealerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerDealerSb->getId(), ActionType::Check});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);
    EXPECT_EQ(myActingPlayersList->size(), 2);

    // Heads-up turn action
    myHand->handlePlayerAction({playerDealerSb->getId(), ActionType::Bet, 60});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Raise, 120});
    myHand->handlePlayerAction({playerDealerSb->getId(), ActionType::Call});

    EXPECT_EQ(myLastGameState, River);
}

} // namespace pkt::test
