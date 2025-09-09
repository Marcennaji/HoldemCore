#include "PostRiverStateTest.h"
#include "core/engine/EngineDefs.h"
#include "core/engine/round_states/PostRiverState.h"
#include "core/engine/round_states/PreflopState.h"
#include "core/player/Helpers.h"
#include "core/services/GlobalServices.h"

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

void PostRiverStateTest::SetUp()
{
    EngineTest::SetUp();
    myEvents.clear();
    myEvents.onBettingRoundStarted = [&](GameState state) { myLastGameState = state; };
}

void PostRiverStateTest::TearDown()
{
    EngineTest::TearDown();
}

void PostRiverStateTest::logTestMessage(const std::string& message)
{
    GlobalServices::instance().logger().info("PostRiverStateTest: " + message);
}

TEST_F(PostRiverStateTest, TerminalStateNoActionsAllowed)
{
    logTestMessage("Testing terminal state - no actions allowed");

    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Reach PostRiver
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, PostRiver);

    // Verify state is terminal
    EXPECT_TRUE(myHandFsm->getState().isTerminal());

    // Verify no further actions are allowed
    EXPECT_FALSE(
        myHandFsm->getActionProcessor()->isActionAllowed(*myHandFsm, {playerSb->getId(), ActionType::Call, 100}));
    EXPECT_FALSE(
        myHandFsm->getActionProcessor()->isActionAllowed(*myHandFsm, {playerBb->getId(), ActionType::Raise, 200}));
}

TEST_F(PostRiverStateTest, ShowdownSingleWinner)
{
    logTestMessage("Testing showdown with single winner");

    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Set different hand rankings (higher = better)
    playerSb->setHandRanking(1000); // Lower ranking
    playerBb->setHandRanking(2000); // Higher ranking (winner)

    // Both players check to showdown
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, PostRiver);

    // Verify BigBlind player is the winner
    auto winners = myHandFsm->getBoard().getWinners();
    EXPECT_EQ(winners.size(), 1);
    EXPECT_TRUE(std::find(winners.begin(), winners.end(), playerBb->getId()) != winners.end());
}

TEST_F(PostRiverStateTest, ShowdownTiedHands)
{
    logTestMessage("Testing showdown with tied hands");

    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Set identical hand rankings
    playerSb->setHandRanking(1500);
    playerBb->setHandRanking(1500); // Same ranking (tie)

    // Both players check to showdown
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, PostRiver);

    // Verify both players are winners
    auto winners = myHandFsm->getBoard().getWinners();
    EXPECT_EQ(winners.size(), 2);
    EXPECT_TRUE(std::find(winners.begin(), winners.end(), playerSb->getId()) != winners.end());
    EXPECT_TRUE(std::find(winners.begin(), winners.end(), playerBb->getId()) != winners.end());
}

TEST_F(PostRiverStateTest, DISABLED_PotDistributionSingleWinner)
{
    logTestMessage("Testing pot distribution to single winner");

    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Set hand rankings
    playerSb->setHandRanking(1000); // Loser
    playerBb->setHandRanking(2000); // Winner

    // Record initial cash
    int initialCashSb = playerSb->getCash();
    int initialCashBb = playerBb->getCash();

    // Both players bet and call to showdown
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 200});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call, 200});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, PostRiver);

    // Verify winner gets the pot (should get back their bet plus opponent's bet)
    EXPECT_EQ(playerBb->getCash(), initialCashBb + 200); // Gets opponent's bet
    EXPECT_EQ(playerSb->getCash(), initialCashSb - 200); // Loses their bet
}

TEST_F(PostRiverStateTest, DISABLED_SplitPotDistribution)
{
    logTestMessage("Testing split pot distribution for tied hands");

    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Set identical hand rankings
    playerSb->setHandRanking(1500);
    playerBb->setHandRanking(1500); // Same ranking

    // Record initial cash
    int initialCashSb = playerSb->getCash();
    int initialCashBb = playerBb->getCash();

    // Both players bet equally to showdown
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 200});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call, 200});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, PostRiver);

    // Verify pot is split equally (both get their money back)
    EXPECT_EQ(playerSb->getCash(), initialCashSb);
    EXPECT_EQ(playerBb->getCash(), initialCashBb);
}

TEST_F(PostRiverStateTest, DISABLED_AllInPlayerWinsEntirePot)
{
    logTestMessage("Testing all-in player wins entire pot");

    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Set up all-in scenario - reduce one player's cash
    playerSb->setCash(300); // Limited cash for all-in

    // All-in player has better hand
    playerSb->setHandRanking(2000); // Winner (all-in)
    playerBb->setHandRanking(1000); // Loser

    // Record initial cash
    int initialCashBb = playerBb->getCash();

    // SB goes all-in, BB calls
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Allin, 250}); // All remaining cash
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call, 250});

    // No more actions possible due to all-in, should go to PostRiver
    EXPECT_EQ(myLastGameState, PostRiver);

    // Verify all-in player wins the pot
    EXPECT_EQ(playerSb->getCash(), 300 + 250);           // Gets back chips plus opponent's call
    EXPECT_EQ(playerBb->getCash(), initialCashBb - 250); // Loses the call amount
}

TEST_F(PostRiverStateTest, DISABLED_FoldedPlayerExcludedFromPot)
{
    logTestMessage("Testing folded player excluded from pot distribution");

    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Set hand rankings - folded player has best hand but shouldn't win
    playerDealer->setHandRanking(3000); // Best hand but will fold
    playerSb->setHandRanking(1000);     // Worse hand
    playerBb->setHandRanking(2000);     // Better of remaining players

    // Record initial cash
    int initialCashDealer = playerDealer->getCash();
    int initialCashSb = playerSb->getCash();
    int initialCashBb = playerBb->getCash();

    // Dealer folds, SB and BB go to showdown
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 200});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call, 200});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, PostRiver);

    // Verify only BB wins (folded player doesn't get pot despite best hand)
    auto winners = myHandFsm->getBoard().getWinners();
    EXPECT_TRUE(std::find(winners.begin(), winners.end(), playerBb->getId()) != winners.end());
    EXPECT_TRUE(std::find(winners.begin(), winners.end(), playerDealer->getId()) == winners.end());

    // Verify pot distribution
    EXPECT_EQ(playerDealer->getCash(), initialCashDealer); // Folded player unchanged
    EXPECT_EQ(playerBb->getCash(), initialCashBb + 200);   // Winner gets pot
    EXPECT_EQ(playerSb->getCash(), initialCashSb - 200);   // Loser loses bet
}

TEST_F(PostRiverStateTest, MultiplePlayersComplexShowdown)
{
    logTestMessage("Testing multiple players complex showdown scenario");

    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Set different hand rankings
    playerDealer->setHandRanking(1000); // Lowest
    playerSb->setHandRanking(1500);     // Middle
    playerBb->setHandRanking(2000);     // Highest (winner)

    // All players call and go to showdown
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, PostRiver);

    // Verify BB player (highest hand) wins
    auto winners = myHandFsm->getBoard().getWinners();
    EXPECT_EQ(winners.size(), 1);
    EXPECT_TRUE(std::find(winners.begin(), winners.end(), playerBb->getId()) != winners.end());
}

TEST_F(PostRiverStateTest, NoNextStateFromPostRiver)
{
    logTestMessage("Testing no next state transition from PostRiver");

    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Reach PostRiver
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, PostRiver);

    // Try any action - should return nullptr for next state
    auto nextState =
        myHandFsm->getActionProcessor()->computeNextState(*myHandFsm, {playerSb->getId(), ActionType::Call, 100});
    EXPECT_EQ(nextState, nullptr);
}

TEST_F(PostRiverStateTest, DISABLED_PotCollectionBeforeDistribution)
{
    logTestMessage("Testing pot collection before distribution");

    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Make some bets during the hand
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call, 100});

    // Check current pot and sets before reaching PostRiver
    int potBeforeRiver = myHandFsm->getBoard().getPot();
    int setsBeforeRiver = myHandFsm->getBoard().getSets();

    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, PostRiver);

    // Verify pot was collected (sets added to pot, sets cleared)
    EXPECT_GT(myHandFsm->getBoard().getPot(), potBeforeRiver);
    EXPECT_EQ(myHandFsm->getBoard().getSets(), 0);
}

TEST_F(PostRiverStateTest, PlayersSetToNoneInPostRiver)
{
    logTestMessage("Testing all players set to ActionType::None in PostRiver");

    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Set players to different actions initially
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call, 100});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, PostRiver);

    // Verify all players are set to ActionType::None
    EXPECT_EQ(playerSb->getLastAction().type, ActionType::None);
    EXPECT_EQ(playerBb->getLastAction().type, ActionType::None);
}

TEST_F(PostRiverStateTest, DISABLED_EmptyPotShowdown)
{
    logTestMessage("Testing showdown with empty pot");

    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Set hand rankings
    playerSb->setHandRanking(1000);
    playerBb->setHandRanking(2000); // Winner

    // Record initial cash
    int initialCashSb = playerSb->getCash();
    int initialCashBb = playerBb->getCash();

    // Both players check through all streets (no betting)
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, PostRiver);

    // Verify winner is still determined even with no side pot
    auto winners = myHandFsm->getBoard().getWinners();
    EXPECT_TRUE(std::find(winners.begin(), winners.end(), playerBb->getId()) != winners.end());

    // Verify no money changes hands (only blinds were collected)
    EXPECT_EQ(playerSb->getCash(), initialCashSb - 50); // Only lost small blind
    EXPECT_EQ(playerBb->getCash(), initialCashBb + 50); // Only won small blind
}

} // namespace pkt::test
