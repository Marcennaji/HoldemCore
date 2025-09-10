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

TEST_F(PostRiverStateTest, PotDistributionSingleWinner)
{
    logTestMessage("Testing pot distribution to single winner");

    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Set hand rankings
    playerSb->setHandRanking(1000); // Loser
    playerBb->setHandRanking(2000); // Winner

    // Record initial cash BEFORE any actions (to capture the true initial state)
    int trueCashSb = 1000; // From gameData.startMoney
    int trueCashBb = 1000; // From gameData.startMoney

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

    // Verify winner gets the pot (should get back their bet plus opponent's bet plus blinds)
    // Total pot calculation from actual contributions:
    // - Preflop: SB posts 10, BB posts 20, SB calls additional 10 to match BB = 20 each total
    // - Flop: SB bets 200, BB calls 200 = 200 each
    // - Total contributions: SB = 220, BB = 220, Total pot = 440
    // Winner (BB) should get entire pot, net change from true initial: +220
    EXPECT_EQ(playerBb->getCash(), trueCashBb + 220); // Net gain from winning entire pot
    EXPECT_EQ(playerSb->getCash(), trueCashSb - 220); // Total loss of their contribution
}

TEST_F(PostRiverStateTest, SplitPotDistribution)
{
    logTestMessage("Testing split pot distribution for tied hands");

    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Set identical hand rankings
    playerSb->setHandRanking(1500);
    playerBb->setHandRanking(1500); // Same ranking

    // Record initial cash BEFORE any actions (to capture the true initial state)
    int trueCashSb = 1000; // From gameData.startMoney
    int trueCashBb = 1000; // From gameData.startMoney

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
    EXPECT_EQ(playerSb->getCash(), trueCashSb);
    EXPECT_EQ(playerBb->getCash(), trueCashBb);
}

TEST_F(PostRiverStateTest, AllInPlayerWinsEntirePot)
{
    logTestMessage("Testing all-in player wins entire pot");

    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Set up all-in scenario - reduce one player's cash AFTER blinds are posted
    // SB has already posted 10, so they have 990 remaining
    playerSb->setCash(270); // This gives them 280 total (10 already posted + 270 remaining)

    // All-in player has better hand
    playerSb->setHandRanking(2000); // Winner (all-in)
    playerBb->setHandRanking(1000); // Loser

    // Record cash after hand start but before all-in
    int cashSbBeforeAllin = playerSb->getCash(); // Should be 270
    int cashBbBeforeAllin = playerBb->getCash(); // Should be 980 (after posting BB)

    // SB goes all-in, BB calls
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Allin, 260}); // All remaining cash after preflop call
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call, 260});

    EXPECT_EQ(myLastGameState, PostRiver);

    // Calculate expected results:
    // SB contributed: 10 (small blind) + 10 (call preflop) + 260 (all-in on flop) = 280 total
    // BB contributed: 20 (big blind) + 0 (check preflop) + 260 (call on flop) = 280 total
    // Total pot = 560, Winner (SB) gets all
    // SB final: 0 (all-in) + 560 (pot) = 560
    // BB final: 1000 - 280 = 720
    EXPECT_EQ(playerSb->getCash(), 560); // All-in amount (0) + winnings (560)
    EXPECT_EQ(playerBb->getCash(), 720); // Initial cash (1000) minus total contribution (280)
}

TEST_F(PostRiverStateTest, FoldedPlayerExcludedFromPot)
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

    // Record initial cash BEFORE any actions (to capture true initial state)
    int trueCashDealer = 1000; // From gameData.startMoney
    int trueCashSb = 1000;     // From gameData.startMoney
    int trueCashBb = 1000;     // From gameData.startMoney

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

    // Verify pot distribution with proper blind accounting:
    // - Dealer: folded immediately, no contribution except any ante (none in this case)
    // - SB: 10 (blind) + 10 (call to complete BB) + 200 (bet) = 220 total contribution
    // - BB: 20 (blind) + 200 (call) = 220 total contribution
    // Total pot: 440, Winner (BB) gets all
    EXPECT_EQ(playerDealer->getCash(), trueCashDealer); // Folded player unchanged
    EXPECT_EQ(playerBb->getCash(), trueCashBb + 220);   // Winner gets 440 - 220 = +220 net
    EXPECT_EQ(playerSb->getCash(), trueCashSb - 220);   // Loser loses their 220 contribution
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

    auto nextState = myHandFsm->getActionProcessor()->computeNextState(*myHandFsm);
    EXPECT_EQ(nextState, nullptr);
}

TEST_F(PostRiverStateTest, PotCollectionBeforeDistribution)
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
    int potBeforeRiver = myHandFsm->getBoard().getPot(*myHandFsm);
    int setsBeforeRiver = myHandFsm->getBoard().getSets(*myHandFsm);

    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, PostRiver);

    // Verify pot collection logic:
    // If there were sets before PostRiver, they should be added to pot and sets cleared
    // If there were no sets, pot should remain the same and sets should still be 0
    if (setsBeforeRiver > 0)
    {
        EXPECT_EQ(myHandFsm->getBoard().getPot(*myHandFsm), potBeforeRiver + setsBeforeRiver);
    }
    else
    {
        EXPECT_EQ(myHandFsm->getBoard().getPot(*myHandFsm), potBeforeRiver);
    }
    EXPECT_EQ(myHandFsm->getBoard().getSets(*myHandFsm), 0);
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

TEST_F(PostRiverStateTest, EmptyPotShowdown)
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
    int initialCashSb = gameData.startMoney; // 1000
    int initialCashBb = gameData.startMoney; // 1000

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

    // Verify no money changes hands except blinds - SB lost 20, BB won 20 (net effect)
    EXPECT_EQ(playerSb->getCash(), initialCashSb - 20); // Lost 20 total (small blind lost, no compensation)
    EXPECT_EQ(playerBb->getCash(), initialCashBb + 20); // Won 20 total (net after losing big blind and winning pot)
}

} // namespace pkt::test
