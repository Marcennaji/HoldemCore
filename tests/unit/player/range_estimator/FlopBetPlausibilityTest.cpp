#include <gtest/gtest.h>
#include "HandPlausibilityTestBase.h"
#include "core/player/range/HandPlausibilityChecker.h"

using namespace pkt::core::player;

namespace pkt::test
{

class FlopBetPlausibilityTest : public HandPlausibilityTestBase
{
  protected:
    void SetUp() override { HandPlausibilityTestBase::SetUp(); }
};

// ========================================
// Maniac Player Exception Tests
// ========================================

TEST_F(FlopBetPlausibilityTest, ManiacPlayerAnyHand_ShouldBePlausible)
{
    // Arrange: Very aggressive (maniac) player with high aggression stats
    auto ctx = createHandContext(true, false);
    setPlayerStatistics(ctx, 4.0f, 60.0f, 100); // Above maniac thresholds (3, 50)

    auto hand = createOvercards(); // Weak hand
    setDrawHeavyBoard(hand);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "Maniac players can bet with any hand - no assumptions made";
}

TEST_F(FlopBetPlausibilityTest, VeryLooseModeAnyHand_ShouldBePlausible)
{
    // Arrange: Player in very loose mode
    auto ctx = createHandContext(true, false);
    setManiacProfile(ctx); // Sets isInVeryLooseMode = true

    auto hand = createOvercards();
    setDryBoard(hand);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "Very loose mode players can bet with any hand - no assumptions made";
}

// ========================================
// Hands Not Using Hole Cards
// ========================================

TEST_F(FlopBetPlausibilityTest, HandNotUsingHoleCards_ShouldBeUnplausible)
{
    // Arrange: Hand doesn't use hole cards
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);

    auto hand = createTopPair(false); // doesn't use hole cards
    validateHoleCardUsage(hand, false);

    // Act & Assert
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "Betting without using hole cards is unplausible";
}

// ========================================
// Donk Betting Out of Position (Not Preflop Aggressor)
// ========================================

TEST_F(FlopBetPlausibilityTest, DonkBetWithOvercards_ShouldBeUnplausible)
{
    // Arrange: Out of position, not preflop aggressor, donk betting with overcards
    auto ctx = createHandContext(false, false); // hasPosition=false
    setLooseAggressiveProfile(ctx);
    ctx.personalContext.actions.preflopIsAggressor = false;
    setMultiPlayerPot(ctx, 3); // Ensure multiway (3+ players)

    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    hand.straightOuts = 0;
    hand.flushOuts = 0;
    // Not a made hand, not a draw
    setDryBoard(hand);

    // Debug output
    std::cout << "DonkBetWithOvercards: hasPosition=" << ctx.personalContext.hasPosition
              << ", preflopIsAggressor=" << ctx.personalContext.actions.preflopIsAggressor
              << ", numPlayers=" << ctx.commonContext.playersContext.nbPlayers << ", isOverCards=" << hand.isOverCards
              << ", isNoPair=" << hand.isNoPair << std::endl;
    // Act & Assert
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "Donk betting with just overcards is unplausible (multiway)";
}

TEST_F(FlopBetPlausibilityTest, DonkBetWithStrongDraw_ShouldBePlausible)
{
    // Arrange: Donk bet with strong draw (8+ outs)
    auto ctx = createHandContext(false, false);
    setLooseAggressiveProfile(ctx);
    ctx.personalContext.actions.preflopIsAggressor = false;

    auto hand = createFlushDraw(9); // 9 outs = strong flush draw
    setFlushDrawBoard(hand);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "Donk betting with strong draw is plausible";
}

TEST_F(FlopBetPlausibilityTest, DonkBetWithNoPair_ShouldBeUnplausible)
{
    // Arrange: Donk bet with no pair, no strong draw
    auto ctx = createHandContext(false, false);
    setLooseAggressiveProfile(ctx);
    ctx.personalContext.actions.preflopIsAggressor = false;
    setMultiPlayerPot(ctx, 3); // Ensure multiway (3+ players)

    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isNoPair = true;
    hand.isOverCards = true;
    hand.straightOuts = 0;
    hand.flushOuts = 0;
    // Not a made hand, not a draw
    setDryBoard(hand);

    // Debug output
    std::cout << "DonkBetWithNoPair: hasPosition=" << ctx.personalContext.hasPosition
              << ", preflopIsAggressor=" << ctx.personalContext.actions.preflopIsAggressor
              << ", numPlayers=" << ctx.commonContext.playersContext.nbPlayers << ", isOverCards=" << hand.isOverCards
              << ", isNoPair=" << hand.isNoPair << std::endl;
    // Act & Assert
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "Donk betting with no pair and weak draws is unplausible (multiway)";
}

TEST_F(FlopBetPlausibilityTest, DISABLED_DonkBetWithWeakPair_ShouldBeUnplausible)
{
    // Arrange: Donk bet with weak pair (not top/middle/over pair)
    auto ctx = createHandContext(false, false);
    setLooseAggressiveProfile(ctx);
    ctx.personalContext.actions.preflopIsAggressor = false;

    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOnePair = true;
    hand.isNoPair = false;
    // Not setting isTopPair, isMiddlePair, or isOverPair = weak pair
    setDryBoard(hand);

    // Act & Assert
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "Donk betting with weak pair is unplausible";
}

TEST_F(FlopBetPlausibilityTest, DonkBetWithTopPair_ShouldBePlausible)
{
    // Arrange: Donk bet with top pair
    auto ctx = createHandContext(false, false);
    setLooseAggressiveProfile(ctx);
    ctx.personalContext.actions.preflopIsAggressor = false;

    auto hand = createTopPair();
    setDryBoard(hand);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "Donk betting with top pair is plausible";
}

TEST_F(FlopBetPlausibilityTest, DonkBetWithSet_ShouldBePlausible)
{
    // Arrange: Donk bet with set
    auto ctx = createHandContext(false, false);
    setLooseAggressiveProfile(ctx);
    ctx.personalContext.actions.preflopIsAggressor = false;

    auto hand = createTrips();
    setDryBoard(hand);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "Donk betting with set is plausible";
}

// ========================================
// In Position Multiway Pot Scenarios
// ========================================

TEST_F(FlopBetPlausibilityTest, InPositionMultiwayWithOvercards_ShouldBeUnplausible)
{
    // Arrange: In position, 3+ players, betting with overcards
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 3);

    auto hand = createOvercards();
    setDryBoard(hand);

    // Act & Assert
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "Betting with just overcards in multiway pot is unplausible";
}

TEST_F(FlopBetPlausibilityTest, InPositionMultiwayWithStrongDraw_ShouldBeUnplausible)
{
    // Arrange: In position multiway with strong draw (should be unplausible)
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);

    auto hand = createFlushDraw(9);
    setFlushDrawBoard(hand);

    // Act & Assert
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "Betting with draws in multiway pot is unplausible";
}

TEST_F(FlopBetPlausibilityTest, InPositionMultiwayWithNoPair_ShouldBeUnplausible)
{
    // Arrange: In position multiway with no pair
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 3);

    auto hand = createOvercards();
    hand.isNoPair = true;
    setDryBoard(hand);

    // Act & Assert
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "Betting with no pair in multiway pot is unplausible";
}

TEST_F(FlopBetPlausibilityTest, InPositionMultiwayWithWeakPair_ShouldBeUnplausible)
{
    // Arrange: In position multiway with weak pair
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 3);

    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOnePair = true;
    hand.isNoPair = false;
    // Not setting top/middle/over pair = weak pair
    setDryBoard(hand);

    // Act & Assert
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "Betting with weak pair in multiway pot is unplausible";
}

TEST_F(FlopBetPlausibilityTest, InPositionMultiwayWithMiddlePair_ShouldBePlausible)
{
    // Arrange: In position multiway with middle pair (should be OK)
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 3);

    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOnePair = true;
    hand.isMiddlePair = true;
    hand.isTopPair = false;
    hand.isOverPair = false;
    hand.straightOuts = 0;
    hand.flushOuts = 0;
    // Not a made hand, not a draw
    setDryBoard(hand);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "Betting with middle pair in position, in multiway pot, is plausible";
}

TEST_F(FlopBetPlausibilityTest, InPositionMultiwayWithTopPair_ShouldBePlausible)
{
    // Arrange: In position multiway with top pair
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);

    auto hand = createTopPair();
    setDryBoard(hand);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "Betting with top pair in multiway pot is plausible";
}

// ========================================
// First to Act Multiway Scenarios
// ========================================

TEST_F(FlopBetPlausibilityTest, FirstToActMultiwayWithNoPair_ShouldBeUnplausible)
{
    // Arrange: First to act (no checks), multiway pot, no pair
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 3);
    setFirstToAct(ctx); // Set first to act scenario

    auto hand = createOvercards();
    hand.isNoPair = true;
    setDryBoard(hand);

    // Act & Assert
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "First to act betting with no pair in multiway pot is unplausible";
}

TEST_F(FlopBetPlausibilityTest, FirstToActMultiwayWithMiddlePair_ShouldBeUnplausible)
{
    // Arrange: Out of position, first to act multiway with middle pair (should need top pair+)
    auto ctx = createHandContext(false, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    setFirstToAct(ctx); // Set first to act scenario

    auto hand = createMiddlePair();
    setDryBoard(hand);

    // Act & Assert
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "First to act betting with middle pair in multiway pot is unplausible";
}

TEST_F(FlopBetPlausibilityTest, FirstToActMultiwayWithTopPair_ShouldBePlausible)
{
    // Arrange: First to act multiway with top pair
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 3);
    setFirstToAct(ctx); // Set first to act scenario

    auto hand = createTopPair();
    setDryBoard(hand);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "First to act betting with top pair in multiway pot is plausible";
}

TEST_F(FlopBetPlausibilityTest, FirstToActMultiwayWithOverPair_ShouldBePlausible)
{
    // Arrange: First to act multiway with overpair
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    setFirstToAct(ctx); // Set first to act scenario

    auto hand = createOverPair();
    setDryBoard(hand);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "First to act betting with overpair in multiway pot is plausible";
}

// ========================================
// Heads-Up Scenarios (More Liberal)
// ========================================

TEST_F(FlopBetPlausibilityTest, HeadsUpWithOvercards_ShouldBePlausible)
{
    // Arrange: Heads-up, can bet wider range
    auto ctx = createHeadsUpContext(true);
    setLooseAggressiveProfile(ctx);

    auto hand = createOvercards();
    setDryBoard(hand);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "Heads-up betting with overcards is plausible";
}

TEST_F(FlopBetPlausibilityTest, HeadsUpWithAnyPair_ShouldBePlausible)
{
    // Arrange: Heads-up with any pair
    auto ctx = createHeadsUpContext(true);
    setLooseAggressiveProfile(ctx);

    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOnePair = true;
    hand.isNoPair = false;
    // Even weak pair should be OK heads-up
    setDryBoard(hand);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "Heads-up betting with any pair is plausible";
}

// ========================================
// Strong Hands Always Plausible
// ========================================

TEST_F(FlopBetPlausibilityTest, StrongHandsAlwaysPlausible_TwoPair)
{
    // Arrange: Two pair (always plausible to bet)
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);

    auto hand = createTwoPair();
    setDryBoard(hand);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "Betting with two pair is always plausible";
}

TEST_F(FlopBetPlausibilityTest, StrongHandsAlwaysPlausible_Set)
{
    // Arrange: Set (always plausible to bet)
    auto ctx = createHandContext(false, false); // Even out of position
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);

    auto hand = createTrips();
    setDryBoard(hand);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "Betting with set is always plausible";
}

TEST_F(FlopBetPlausibilityTest, StrongHandsAlwaysPlausible_Straight)
{
    // Arrange: Straight (always plausible to bet)
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);

    auto hand = createStraight();
    setFlushDrawBoard(hand);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "Betting with straight is always plausible";
}

TEST_F(FlopBetPlausibilityTest, StrongHandsAlwaysPlausible_Flush)
{
    // Arrange: Flush (always plausible to bet)
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);

    auto hand = createFlush();
    setStraightDrawBoard(hand);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(hand, ctx))
        << "Betting with flush is always plausible";
}

} // namespace pkt::test