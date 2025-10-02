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
    m_events.clear();
    m_events.onBettingRoundStarted = [&](GameState state) { m_lastGameState = state; };
}

void TurnStateTest::TearDown()
{
    EngineTest::TearDown();
}

TEST_F(TurnStateTest, StartTurnInitializesPlayersCorrectly)
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

    // Verify players are still active and have valid cash amounts
    EXPECT_EQ(isPlayerStillActive(playerDealer->getId()), true);
    EXPECT_EQ(isPlayerStillActive(playerBb->getId()), true);
    EXPECT_GE(playerDealer->getCash(), 0);
    EXPECT_GE(playerBb->getCash(), 0);
}

TEST_F(TurnStateTest, OnlyChecksOnTurnShouldGoToRiver)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Progress through preflop and flop
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Turn);

    // Turn actions - all checks should progress to River
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, River);
}

TEST_F(TurnStateTest, BetOnTurnKeepsRoundOpen)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Progress through preflop and flop
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Turn);

    // Turn bet scenario
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    EXPECT_EQ(m_lastGameState, Turn); // round still open

    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Call});
    EXPECT_EQ(m_lastGameState, River); // round closes after call
}

TEST_F(TurnStateTest, RaiseOnTurnKeepsRoundOpen)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Progress through preflop and flop
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Turn);

    // Turn raise scenario
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 75});
    EXPECT_EQ(m_lastGameState, Turn); // round still open

    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Raise, 150});
    EXPECT_EQ(m_lastGameState, Turn); // round still open after raise

    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    EXPECT_EQ(m_lastGameState, River); // round closes after call
}

TEST_F(TurnStateTest, FoldOnTurnWithTwoPlayersEndsHand)
{
    initializeHandWithPlayers(2, gameData);

    auto playerDealerSb = getPlayerById(m_actingPlayersList, 0);
    auto playerBb = getPlayerById(m_actingPlayersList, 1);

    // Progress through preflop and flop
    m_hand->handlePlayerAction({playerDealerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerDealerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Turn);

    // One player bets, other folds - should end hand
    m_hand->handlePlayerAction({playerDealerSb->getId(), ActionType::Bet, 80});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Fold});

    // Hand should end with only one player remaining
    EXPECT_EQ(m_actingPlayersList->size(), 1);
    EXPECT_EQ(m_lastGameState, PostRiver); // Hand ends, no further rounds
}

TEST_F(TurnStateTest, AllInOnTurn)
{
    initializeHandWithPlayers(2, gameData);

    auto playerDealerSb = getPlayerById(m_actingPlayersList, 0);
    auto playerBb = getPlayerById(m_actingPlayersList, 1);

    // Progress through preflop and flop
    m_hand->handlePlayerAction({playerDealerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerDealerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Turn);

    // Player goes all-in on turn
    m_hand->handlePlayerAction({playerDealerSb->getId(), ActionType::Allin});
    EXPECT_EQ(getPlayerById(m_seatsList, playerDealerSb->getId())->getCash(), 0);
    EXPECT_EQ(m_lastGameState, Turn); // Round still open after all-in
}

TEST_F(TurnStateTest, MultipleFoldsOnTurn)
{
    initializeHandWithPlayers(4, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);
    auto playerUtg = getPlayerById(m_actingPlayersList, 3);

    // Progress through preflop and flop with all players
    m_hand->handlePlayerAction({playerUtg->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    // Flop - all check
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerUtg->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Turn);
    EXPECT_EQ(m_actingPlayersList->size(), 4);

    // Multiple folds on turn
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 120});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Fold});
    EXPECT_EQ(m_actingPlayersList->size(), 3);

    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    EXPECT_EQ(m_actingPlayersList->size(), 2);

    m_hand->handlePlayerAction({playerUtg->getId(), ActionType::Call});
    EXPECT_EQ(m_lastGameState, River); // Round should advance to River
}

TEST_F(TurnStateTest, CheckRaiseScenarioOnTurn)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Progress through preflop and flop
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Turn);

    // Check-raise scenario on turn
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Bet, 100});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Raise, 200});

    EXPECT_EQ(m_lastGameState, Turn); // round still open after raise

    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Call});
    EXPECT_EQ(m_lastGameState, River); // round closes after call
}

TEST_F(TurnStateTest, AllInCallScenarioOnTurn)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Progress through preflop and flop
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Turn);

    // One player goes all-in, other calls
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Allin});
    EXPECT_EQ(getPlayerById(m_seatsList, playerSb->getId())->getCash(), 0);

    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Call});

    // Both players are all-in, but should advance to River first (not skip directly to PostRiver)
    EXPECT_EQ(m_lastGameState, River);
}

TEST_F(TurnStateTest, BetFoldScenarioOnTurn)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Progress through preflop and flop
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Turn);
    size_t initialActingPlayers = m_actingPlayersList->size();

    // Bet and fold scenario
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 150});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Fold});

    // One player should have folded
    EXPECT_EQ(m_actingPlayersList->size(), initialActingPlayers - 1);
    EXPECT_EQ(m_lastGameState, PostRiver); // Hand ends, no further rounds
}

TEST_F(TurnStateTest, LargeBetOnTurnRequiresResponse)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Progress through preflop and flop
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Turn);

    // Large bet on turn
    int initialCash = playerSb->getCash();
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 300});

    // Verify bet was applied
    EXPECT_LT(playerSb->getCash(), initialCash);
    EXPECT_EQ(m_lastGameState, Turn); // round still open, awaiting response
}

TEST_F(TurnStateTest, MultipleRaisesOnTurn)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Progress through preflop and flop
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Turn);

    // Multiple raises scenario
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Bet, 50});
    EXPECT_EQ(m_lastGameState, Turn); // round still open

    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Raise, 100});
    EXPECT_EQ(m_lastGameState, Turn); // round still open after first raise

    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Raise, 200});
    EXPECT_EQ(m_lastGameState, Turn); // round still open after second raise

    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Call});
    EXPECT_EQ(m_lastGameState, River); // round closes after call
}

TEST_F(TurnStateTest, HeadsUpTurnAction)
{
    initializeHandWithPlayers(2, gameData);

    auto playerDealerSb = getPlayerById(m_actingPlayersList, 0);
    auto playerBb = getPlayerById(m_actingPlayersList, 1);

    // Progress through preflop and flop
    m_hand->handlePlayerAction({playerDealerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerDealerSb->getId(), ActionType::Check});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Turn);
    EXPECT_EQ(m_actingPlayersList->size(), 2);

    // Heads-up turn action
    m_hand->handlePlayerAction({playerDealerSb->getId(), ActionType::Bet, 60});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Raise, 120});
    m_hand->handlePlayerAction({playerDealerSb->getId(), ActionType::Call});

    EXPECT_EQ(m_lastGameState, River);
}

} // namespace pkt::test
