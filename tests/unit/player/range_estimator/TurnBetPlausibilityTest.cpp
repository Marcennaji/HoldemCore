#include <gtest/gtest.h>
#include "HandPlausibilityTestBase.h"
#include "core/player/range/HandPlausibilityChecker.h"

using namespace pkt::core::player;

namespace pkt::test
{

class TurnBetPlausibilityTest : public HandPlausibilityTestBase
{
  protected:
    void SetUp() override { HandPlausibilityTestBase::SetUp(); }
};

// ========================================
// Maniac Player Exception Tests
// ========================================

TEST_F(TurnBetPlausibilityTest, ManiacPlayerAnyHand_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setPlayerStatistics(ctx, 4.0f, 60.0f, 100);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnBet(hand, ctx))
        << "Maniac players can bet with any hand - no assumptions made";
}

TEST_F(TurnBetPlausibilityTest, VeryLooseModeAnyHand_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setManiacProfile(ctx);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnBet(hand, ctx))
        << "Very loose mode players can bet with any hand - no assumptions made";
}

// ========================================
// Hands Not Using Hole Cards
// ========================================

TEST_F(TurnBetPlausibilityTest, HandNotUsingHoleCards_ShouldBeUnplausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    auto hand = createTopPair(false);
    validateHoleCardUsage(hand, false);
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenTurnBet(hand, ctx))
        << "Betting without using hole cards is unplausible";
}

// ========================================
// Donk Betting Out of Position (Not Flop Aggressor)
// ========================================

TEST_F(TurnBetPlausibilityTest, DonkBetWithOvercards_ShouldBeUnplausible)
{
    auto ctx = createHandContext(false, false);
    setLooseAggressiveProfile(ctx);
    ctx.personalContext.actions.flopIsAggressor = false;
    ctx.commonContext.bettingContext.flopBetsOrRaisesNumber = 1;
    setMultiPlayerPot(ctx, 3);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    hand.straightOuts = 0;
    hand.flushOuts = 0;
    setDryBoard(hand);
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenTurnBet(hand, ctx))
        << "Donk betting with just overcards is unplausible (multiway)";
}

TEST_F(TurnBetPlausibilityTest, DonkBetWithStrongDraw_ShouldNotBePlausible)
{
    auto ctx = createHandContext(false, false);
    setLooseAggressiveProfile(ctx);
    ctx.personalContext.actions.flopIsAggressor = false;
    ctx.commonContext.bettingContext.flopBetsOrRaisesNumber = 1;
    auto hand = createFlushDraw(9);
    setFlushDrawBoard(hand);
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenTurnBet(hand, ctx))
        << "Donk betting with strong draw is not plausible";
}

TEST_F(TurnBetPlausibilityTest, DonkBetWithNoPair_ShouldBeUnplausible)
{
    auto ctx = createHandContext(false, false);
    setLooseAggressiveProfile(ctx);
    ctx.personalContext.actions.flopIsAggressor = false;
    ctx.commonContext.bettingContext.flopBetsOrRaisesNumber = 1;
    setMultiPlayerPot(ctx, 3);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isNoPair = true;
    hand.isOverCards = true;
    hand.straightOuts = 0;
    hand.flushOuts = 0;
    setDryBoard(hand);
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenTurnBet(hand, ctx))
        << "Donk betting with no pair and weak draws is unplausible (multiway)";
}

// ========================================
// In Position Multiway Pot Scenarios
// ========================================

TEST_F(TurnBetPlausibilityTest, InPositionMultiwayWithOvercards_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 3);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnBet(hand, ctx))
        << "Betting with just overcards in multiway pot is not unplausible";
}

TEST_F(TurnBetPlausibilityTest, InPositionMultiwayWithStrongDraw_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createFlushDraw(9);
    setFlushDrawBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnBet(hand, ctx))
        << "Betting with draws in multiway pot is plausible";
}

TEST_F(TurnBetPlausibilityTest, InPositionMultiwayWithNoPair_ShouldBeUnplausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 3);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isNoPair = true;
    hand.isOverCards = true;
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnBet(hand, ctx))
        << "Betting with no pair in multiway pot is plausible";
}

TEST_F(TurnBetPlausibilityTest, InPositionMultiwayWithWeakPair_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 3);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOnePair = true;
    hand.isNoPair = false;
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnBet(hand, ctx))
        << "Betting with weak pair in multiway pot is plausible";
}

TEST_F(TurnBetPlausibilityTest, InPositionMultiwayWithMiddlePair_ShouldBePlausible)
{
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
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnBet(hand, ctx))
        << "Betting with middle pair in multiway pot is plausible";
}

TEST_F(TurnBetPlausibilityTest, InPositionMultiwayWithTopPair_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createTopPair();
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnBet(hand, ctx))
        << "Betting with top pair in multiway pot is plausible";
}

// ========================================
// Strong Hands Always Plausible
// ========================================

TEST_F(TurnBetPlausibilityTest, StrongHandsAlwaysPlausible_TwoPair)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createTwoPair();
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnBet(hand, ctx))
        << "Betting with two pair is always plausible";
}

TEST_F(TurnBetPlausibilityTest, StrongHandsAlwaysPlausible_Set)
{
    auto ctx = createHandContext(false, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createTrips();
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnBet(hand, ctx))
        << "Betting with set is always plausible";
}

TEST_F(TurnBetPlausibilityTest, StrongHandsAlwaysPlausible_Straight)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    auto hand = createStraight();
    setFlushDrawBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnBet(hand, ctx))
        << "Betting with straight is always plausible";
}

TEST_F(TurnBetPlausibilityTest, StrongHandsAlwaysPlausible_Flush)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    auto hand = createFlush();
    setStraightDrawBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnBet(hand, ctx))
        << "Betting with flush is always plausible";
}

} // namespace pkt::test
