// tests/TurnStateTest.cpp

#include "TurnStateTest.h"
#include "core/engine/model/PlayerAction.h"
#include "core/engine/round_states/TurnState.h"
#include "core/player/Helpers.h"
#include "core/services/GlobalServices.h"

using namespace pkt::core;
using namespace pkt::core::player;
using namespace std;

namespace pkt::test
{

void TurnStateTest::logTestMessage(const std::string& message) const
{
    GlobalServices::instance().logger().verbose("TurnState : " + message);
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
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

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

    // Verify players are still active and have valid cash amounts
    EXPECT_EQ(isPlayerStillActive(playerDealer->getId()), true);
    EXPECT_EQ(isPlayerStillActive(playerBb->getId()), true);
    EXPECT_GE(playerDealer->getCash(), 0);
    EXPECT_GE(playerBb->getCash(), 0);
}

TEST_F(TurnStateTest, OnlyChecksOnTurnShouldGoToRiver)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Progress through preflop and flop
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);

    // Turn actions - all checks should progress to River
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, River);
}

TEST_F(TurnStateTest, BetOnTurnKeepsRoundOpen)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Progress through preflop and flop
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);

    // Turn bet scenario
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    EXPECT_EQ(myLastGameState, Turn); // round still open

    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, River); // round closes after call
}

TEST_F(TurnStateTest, RaiseOnTurnKeepsRoundOpen)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Progress through preflop and flop
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);

    // Turn raise scenario
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 75});
    EXPECT_EQ(myLastGameState, Turn); // round still open

    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Raise, 150});
    EXPECT_EQ(myLastGameState, Turn); // round still open after raise

    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, River); // round closes after call
}

TEST_F(TurnStateTest, FoldOnTurnWithTwoPlayersEndsHand)
{
    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    auto playerDealerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Progress through preflop and flop
    myHandFsm->handlePlayerAction({playerDealerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerDealerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);

    // One player bets, other folds - should end hand
    myHandFsm->handlePlayerAction({playerDealerSb->getId(), ActionType::Bet, 80});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Fold});

    // Hand should end with only one player remaining
    EXPECT_EQ(myActingPlayersListFsm->size(), 1);
    EXPECT_EQ(myLastGameState, PostRiver); // Hand ends, no further rounds
}

TEST_F(TurnStateTest, AllInOnTurn)
{
    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    auto playerDealerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Progress through preflop and flop
    myHandFsm->handlePlayerAction({playerDealerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerDealerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);

    // Player goes all-in on turn
    myHandFsm->handlePlayerAction({playerDealerSb->getId(), ActionType::Allin});
    EXPECT_EQ(getPlayerFsmById(mySeatsListFsm, playerDealerSb->getId())->getCash(), 0);
    EXPECT_EQ(myLastGameState, Turn); // Round still open after all-in
}

TEST_F(TurnStateTest, MultipleFoldsOnTurn)
{
    initializeHandFsmForTesting(4, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);
    auto playerUtg = getPlayerFsmById(myActingPlayersListFsm, 3);

    // Progress through preflop and flop with all players
    myHandFsm->handlePlayerAction({playerUtg->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    // Flop - all check
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerUtg->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);
    EXPECT_EQ(myActingPlayersListFsm->size(), 4);

    // Multiple folds on turn
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 120});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Fold});
    EXPECT_EQ(myActingPlayersListFsm->size(), 3);

    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    EXPECT_EQ(myActingPlayersListFsm->size(), 2);

    myHandFsm->handlePlayerAction({playerUtg->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, River); // Round should advance to River
}

TEST_F(TurnStateTest, CheckRaiseScenarioOnTurn)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Progress through preflop and flop
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);

    // Check-raise scenario on turn
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Bet, 100});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Raise, 200});

    EXPECT_EQ(myLastGameState, Turn); // round still open after raise

    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, River); // round closes after call
}

TEST_F(TurnStateTest, AllInCallScenarioOnTurn)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Progress through preflop and flop
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);

    // One player goes all-in, other calls
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Allin});
    EXPECT_EQ(getPlayerFsmById(mySeatsListFsm, playerSb->getId())->getCash(), 0);

    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call});

    // Both players should be allin, round should advance
    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(TurnStateTest, BetFoldScenarioOnTurn)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Progress through preflop and flop
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);
    size_t initialActingPlayers = myActingPlayersListFsm->size();

    // Bet and fold scenario
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 150});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Fold});

    // One player should have folded
    EXPECT_EQ(myActingPlayersListFsm->size(), initialActingPlayers - 1);
    EXPECT_EQ(myLastGameState, PostRiver); // Hand ends, no further rounds
}

TEST_F(TurnStateTest, LargeBetOnTurnRequiresResponse)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Progress through preflop and flop
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);

    // Large bet on turn
    int initialCash = playerSb->getCash();
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 300});

    // Verify bet was applied
    EXPECT_LT(playerSb->getCash(), initialCash);
    EXPECT_EQ(myLastGameState, Turn); // round still open, awaiting response
}

TEST_F(TurnStateTest, MultipleRaisesOnTurn)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Progress through preflop and flop
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);

    // Multiple raises scenario
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 50});
    EXPECT_EQ(myLastGameState, Turn); // round still open

    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Raise, 100});
    EXPECT_EQ(myLastGameState, Turn); // round still open after first raise

    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Raise, 200});
    EXPECT_EQ(myLastGameState, Turn); // round still open after second raise

    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call});
    EXPECT_EQ(myLastGameState, River); // round closes after call
}

TEST_F(TurnStateTest, HeadsUpTurnAction)
{
    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    auto playerDealerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Progress through preflop and flop
    myHandFsm->handlePlayerAction({playerDealerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerDealerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Turn);
    EXPECT_EQ(myActingPlayersListFsm->size(), 2);

    // Heads-up turn action
    myHandFsm->handlePlayerAction({playerDealerSb->getId(), ActionType::Bet, 60});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Raise, 120});
    myHandFsm->handlePlayerAction({playerDealerSb->getId(), ActionType::Call});

    EXPECT_EQ(myLastGameState, River);
}

} // namespace pkt::test
