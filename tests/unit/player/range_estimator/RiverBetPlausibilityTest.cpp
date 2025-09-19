#include <gtest/gtest.h>
#include "HandPlausibilityTestBase.h"
#include "core/player/range/HandPlausibilityChecker.h"

using namespace pkt::core::player;

namespace pkt::test
{

class RiverBetPlausibilityTest : public HandPlausibilityTestBase
{
  protected:
    void SetUp() override { HandPlausibilityTestBase::SetUp(); }
};

// ========================================
// Maniac Player Exception Tests
// ========================================

TEST_F(RiverBetPlausibilityTest, ManiacPlayerAnyHand_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setPlayerStatistics(ctx, 4.0f, 60.0f, 100);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverBet(hand, ctx))
        << "Maniac players can bet river with any hand - no assumptions made";
}

TEST_F(RiverBetPlausibilityTest, VeryLooseModeAnyHand_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setManiacProfile(ctx);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverBet(hand, ctx))
        << "Very loose mode players can bet river with any hand - no assumptions made";
}

// ========================================
// Hands Not Using Hole Cards
// ========================================

TEST_F(RiverBetPlausibilityTest, HandNotUsingHoleCards_ShouldBeUnplausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    auto hand = createTopPair(false);
    validateHoleCardUsage(hand, false);
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenRiverBet(hand, ctx))
        << "Betting river without using hole cards is unplausible";
}

// ========================================
// Multiway, Out of Position, Action Scenario
// ========================================

TEST_F(RiverBetPlausibilityTest, MultiwayOutOfPositionWithWeakHand_ShouldBeUnplausible)
{
    auto ctx = createHandContext(false, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    ctx.commonContext.bettingContext.flopBetsOrRaisesNumber = 2;
    ctx.commonContext.bettingContext.turnBetsOrRaisesNumber = 2;
    ctx.personalContext.actions.flopIsAggressor = false;
    ctx.personalContext.actions.turnIsAggressor = false;
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isNoPair = true;
    hand.isOverCards = true;
    setDryBoard(hand);
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenRiverBet(hand, ctx))
        << "Betting river out of position with weak hand in multiway/action pot is unplausible";
}

TEST_F(RiverBetPlausibilityTest, MultiwayOutOfPositionWithTwoPair_ShouldBePlausible)
{
    auto ctx = createHandContext(false, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    ctx.commonContext.bettingContext.flopBetsOrRaisesNumber = 2;
    ctx.commonContext.bettingContext.turnBetsOrRaisesNumber = 2;
    ctx.personalContext.actions.flopIsAggressor = false;
    ctx.personalContext.actions.turnIsAggressor = false;
    auto hand = createTwoPair();
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverBet(hand, ctx))
        << "Betting river out of position with two pair in multiway/action pot is plausible";
}

// ========================================
// Strong Hands Always Plausible
// ========================================

TEST_F(RiverBetPlausibilityTest, StrongHandsAlwaysPlausible_Set)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    auto hand = createTrips();
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverBet(hand, ctx))
        << "Betting river with set is always plausible";
}

TEST_F(RiverBetPlausibilityTest, StrongHandsAlwaysPlausible_Straight)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    auto hand = createStraight();
    setFlushDrawBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverBet(hand, ctx))
        << "Betting river with straight is always plausible";
}

TEST_F(RiverBetPlausibilityTest, StrongHandsAlwaysPlausible_Flush)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    auto hand = createFlush();
    setStraightDrawBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverBet(hand, ctx))
        << "Betting river with flush is always plausible";
}

} // namespace pkt::test
