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

    initializeHandFsmWithPlayers(2, gameData);

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
    EXPECT_FALSE(myHandFsm->getActionProcessor()->isActionAllowed(*myHandFsm, {playerSb->getId(), ActionType::Call}));
    EXPECT_FALSE(
        myHandFsm->getActionProcessor()->isActionAllowed(*myHandFsm, {playerBb->getId(), ActionType::Raise, 200}));
}

TEST_F(PostRiverStateTest, ShowdownSingleWinner)
{
    logTestMessage("Testing showdown with single winner");

    initializeHandFsmWithPlayers(2, gameData);

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

    initializeHandFsmWithPlayers(2, gameData);

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

    initializeHandFsmWithPlayers(2, gameData);

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
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call});
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

    initializeHandFsmWithPlayers(2, gameData);

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
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call});
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

    initializeHandFsmWithPlayers(2, gameData);

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
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call});

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

    initializeHandFsmWithPlayers(3, gameData);

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
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call});
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

    initializeHandFsmWithPlayers(3, gameData);

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

    initializeHandFsmWithPlayers(2, gameData);

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

    initializeHandFsmWithPlayers(2, gameData);

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Make some bets during the hand
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call});

    // Check current pot and sets before reaching PostRiver
    int potBeforeRiver = myHandFsm->getBoard().getPot(*myHandFsm);
    int setsBeforeRiver = myHandFsm->getBoard().getSets(*myHandFsm);

    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, PostRiver);

    // After pot distribution in PostRiver state:
    // 1. The pot should be 0 (money has been distributed to winners)
    // 2. Sets should be 0 (any remaining sets were added to pot before distribution)
    EXPECT_EQ(myHandFsm->getBoard().getPot(*myHandFsm), 0) << "Pot should be 0 after distribution";
    EXPECT_EQ(myHandFsm->getBoard().getSets(*myHandFsm), 0) << "Sets should be 0 after collection";

    // Verify that money was actually distributed (pot wasn't just lost)
    // Total pot before distribution should have been: potBeforeRiver + setsBeforeRiver
    int expectedTotalPot = potBeforeRiver + setsBeforeRiver;
    EXPECT_GT(expectedTotalPot, 0) << "There should have been a pot to distribute";

    // The money should now be in the players' cash (can't easily verify exact amounts
    // without knowing the winner, but we can verify pot clearing worked correctly)
}

TEST_F(PostRiverStateTest, PlayersSetToNoneInPostRiver)
{
    logTestMessage("Testing all players set to ActionType::None in PostRiver");

    initializeHandFsmWithPlayers(2, gameData);

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Set players to different actions initially
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 100});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, PostRiver);

    // Verify all players are set to ActionType::None
    EXPECT_EQ(playerSb->getLastAction().type, ActionType::None);
    EXPECT_EQ(playerBb->getLastAction().type, ActionType::None);
}

TEST_F(PostRiverStateTest, PotClearedAfterDistribution)
{
    logTestMessage("Testing pot is properly cleared after distribution");

    initializeHandFsmWithPlayers(2, gameData);

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Set hand rankings to ensure deterministic winner
    playerSb->setHandRanking(1000); // Loser
    playerBb->setHandRanking(2000); // Winner

    // Record initial cash at hand start (before any betting)
    int initialCashSb = playerSb->getCashAtHandStart();
    int initialCashBb = playerBb->getCashAtHandStart();

    // Create a substantial pot
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Bet, 300});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Check});
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, PostRiver);

    // After PostRiver state is complete:
    // 1. Pot should be 0 (cleared after distribution)
    EXPECT_EQ(myHandFsm->getBoard().getPot(*myHandFsm), 0);

    // 2. Sets should be 0 (cleared after collection)
    EXPECT_EQ(myHandFsm->getBoard().getSets(*myHandFsm), 0);

    // 3. Money should have been properly distributed to winner
    // Let's calculate the net change for each player (more robust than absolute values)
    int netChangeBb = playerBb->getCash() - initialCashBb;
    int netChangeSb = playerSb->getCash() - initialCashSb;

    // The net changes should sum to 0 (conservation of money)
    EXPECT_EQ(netChangeBb + netChangeSb, 0) << "Money should be conserved (total change = 0)";

    // Winner (BB) should have gained money, loser (SB) should have lost the same amount
    EXPECT_GT(netChangeBb, 0) << "Winner should have gained money";
    EXPECT_LT(netChangeSb, 0) << "Loser should have lost money";
    EXPECT_EQ(netChangeBb, -netChangeSb) << "Winner's gain should equal loser's loss";

    // 4. Player betting actions should be cleared (this is what enables pot = 0)
    EXPECT_EQ(playerSb->getCurrentHandActions().getHandTotalBetAmount(), 0);
    EXPECT_EQ(playerBb->getCurrentHandActions().getHandTotalBetAmount(), 0);
}

TEST_F(PostRiverStateTest, EmptyPotShowdown)
{
    logTestMessage("Testing showdown with empty pot");

    initializeHandFsmWithPlayers(2, gameData);

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
