#include <gtest/gtest.h>
#include "HandPlausibilityTestBase.h"
#include "core/player/range/HandPlausibilityChecker.h"

using namespace pkt::core::player;

namespace pkt::test
{

class RiverCallPlausibilityTest : public HandPlausibilityTestBase
{
  protected:
    void SetUp() override { HandPlausibilityTestBase::SetUp(); }
};

// ========================================
// Maniac Player Exception Tests
// ========================================

TEST_F(RiverCallPlausibilityTest, ManiacPlayerAnyHand_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setPlayerStatistics(ctx, 4.0f, 60.0f, 100);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverCall(hand, ctx))
        << "Maniac players can call river with any hand - no assumptions made";
}

TEST_F(RiverCallPlausibilityTest, VeryLooseModeAnyHand_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setManiacProfile(ctx);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverCall(hand, ctx))
        << "Very loose mode players can call river with any hand - no assumptions made";
}

// ========================================
// Hands Not Using Hole Cards
// ========================================

TEST_F(RiverCallPlausibilityTest, DISABLED_HandNotUsingHoleCards_ShouldBeUnplausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    auto hand = createTopPair(false);
    validateHoleCardUsage(hand, false);
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenRiverCall(hand, ctx))
        << "Calling river without using hole cards is unplausible";
}

// ========================================
// Multiway Pot Scenarios
// ========================================

TEST_F(RiverCallPlausibilityTest, DISABLED_MultiwayWithNoPair_ShouldBeUnplausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isNoPair = true;
    hand.isOverCards = true;
    setDryBoard(hand);
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenRiverCall(hand, ctx))
        << "Calling river with no pair in multiway pot is unplausible";
}

TEST_F(RiverCallPlausibilityTest, DISABLED_MultiwayWithWeakPair_ShouldBeUnplausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOnePair = true;
    hand.isNoPair = false;
    setDryBoard(hand);
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenRiverCall(hand, ctx))
        << "Calling river with weak pair in multiway pot is unplausible";
}

TEST_F(RiverCallPlausibilityTest, MultiwayWithTopPair_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createTopPair();
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverCall(hand, ctx))
        << "Calling river with top pair in multiway pot is plausible";
}

// ========================================
// Strong Hands Always Plausible
// ========================================

TEST_F(RiverCallPlausibilityTest, StrongHandsAlwaysPlausible_TwoPair)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createTwoPair();
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverCall(hand, ctx))
        << "Calling river with two pair is always plausible";
}

TEST_F(RiverCallPlausibilityTest, StrongHandsAlwaysPlausible_Set)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createTrips();
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverCall(hand, ctx))
        << "Calling river with set is always plausible";
}

TEST_F(RiverCallPlausibilityTest, StrongHandsAlwaysPlausible_Straight)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createStraight();
    setFlushDrawBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverCall(hand, ctx))
        << "Calling river with straight is always plausible";
}

TEST_F(RiverCallPlausibilityTest, StrongHandsAlwaysPlausible_Flush)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createFlush();
    setStraightDrawBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverCall(hand, ctx))
        << "Calling river with flush is always plausible";
}

} // namespace pkt::test
