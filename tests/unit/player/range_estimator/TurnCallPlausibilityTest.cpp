#include <gtest/gtest.h>
#include "HandPlausibilityTestBase.h"
#include "core/player/range/HandPlausibilityChecker.h"

using namespace pkt::core::player;

namespace pkt::test
{

class TurnCallPlausibilityTest : public HandPlausibilityTestBase
{
  protected:
    void SetUp() override { HandPlausibilityTestBase::SetUp(); }
};

// ========================================
// Maniac Player Exception Tests
// ========================================

TEST_F(TurnCallPlausibilityTest, ManiacPlayerAnyHand_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setPlayerStatistics(ctx, 4.0f, 60.0f, 100);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnCall(hand, ctx))
        << "Maniac players can call with any hand - no assumptions made";
}

TEST_F(TurnCallPlausibilityTest, VeryLooseModeAnyHand_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setManiacProfile(ctx);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnCall(hand, ctx))
        << "Very loose mode players can call with any hand - no assumptions made";
}

// ========================================
// Hands Not Using Hole Cards
// ========================================

TEST_F(TurnCallPlausibilityTest, HandNotUsingHoleCards_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    auto hand = createTopPair(false);
    validateHoleCardUsage(hand, false);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnCall(hand, ctx))
        << "Calling without using hole cards is plausible";
}

// ========================================
// Multiway Pot Scenarios
// ========================================

TEST_F(TurnCallPlausibilityTest, MultiwayWithNoPair_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 3);
    ctx.commonContext.bettingContext.turnBetsOrRaisesNumber = 1;
    ctx.commonContext.bettingContext.flopBetsOrRaisesNumber = 1;
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isNoPair = true;
    hand.isOverCards = true;
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnCall(hand, ctx))
        << "Calling with no pair in multiway pot is plausible";
}

TEST_F(TurnCallPlausibilityTest, MultiwayWithWeakPair_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 3);
    ctx.commonContext.bettingContext.turnBetsOrRaisesNumber = 1;
    ctx.commonContext.bettingContext.flopBetsOrRaisesNumber = 1;
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOnePair = true;
    hand.isNoPair = false;
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnCall(hand, ctx))
        << "Calling with weak pair in multiway pot is plausible";
}

TEST_F(TurnCallPlausibilityTest, MultiwayWithTopPair_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 3);
    ctx.commonContext.bettingContext.turnBetsOrRaisesNumber = 1;
    ctx.commonContext.bettingContext.flopBetsOrRaisesNumber = 1;
    auto hand = createTopPair();
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnCall(hand, ctx))
        << "Calling with top pair in multiway pot is plausible";
}

// ========================================
// Strong Hands Always Plausible
// ========================================

TEST_F(TurnCallPlausibilityTest, StrongHandsAlwaysPlausible_TwoPair)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 3);
    ctx.commonContext.bettingContext.turnBetsOrRaisesNumber = 1;
    ctx.commonContext.bettingContext.flopBetsOrRaisesNumber = 1;
    auto hand = createTwoPair();
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnCall(hand, ctx))
        << "Calling with two pair is always plausible";
}

TEST_F(TurnCallPlausibilityTest, StrongHandsAlwaysPlausible_Set)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 3);
    ctx.commonContext.bettingContext.turnBetsOrRaisesNumber = 1;
    ctx.commonContext.bettingContext.flopBetsOrRaisesNumber = 1;
    auto hand = createTrips();
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnCall(hand, ctx))
        << "Calling with set is always plausible";
}

TEST_F(TurnCallPlausibilityTest, StrongHandsAlwaysPlausible_Straight)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 3);
    ctx.commonContext.bettingContext.turnBetsOrRaisesNumber = 1;
    ctx.commonContext.bettingContext.flopBetsOrRaisesNumber = 1;
    auto hand = createStraight();
    setFlushDrawBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnCall(hand, ctx))
        << "Calling with straight is always plausible";
}

TEST_F(TurnCallPlausibilityTest, StrongHandsAlwaysPlausible_Flush)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 3);
    ctx.commonContext.bettingContext.turnBetsOrRaisesNumber = 1;
    ctx.commonContext.bettingContext.flopBetsOrRaisesNumber = 1;
    auto hand = createFlush();
    setStraightDrawBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnCall(hand, ctx))
        << "Calling with flush is always plausible";
}

} // namespace pkt::test
