#include <gtest/gtest.h>
#include "HandPlausibilityTestBase.h"
#include "core/player/range/HandPlausibilityChecker.h"

using namespace pkt::core::player;

namespace pkt::test
{

class FlopRaisePlausibilityTest : public HandPlausibilityTestBase
{
  protected:
    void SetUp() override { HandPlausibilityTestBase::SetUp(); }
};

// ========================================
// Maniac Player Exception Tests
// ========================================

TEST_F(FlopRaisePlausibilityTest, ManiacPlayerAnyHand_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setPlayerStatistics(ctx, 4.0f, 60.0f, 100);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Flop,
                                                             {ctx.personalContext.id, ActionType::Raise});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopRaise(hand, ctx))
        << "Maniac players can raise flop with any hand - no assumptions made";
}

TEST_F(FlopRaisePlausibilityTest, VeryLooseModeAnyHand_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setManiacProfile(ctx);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Flop,
                                                             {ctx.personalContext.id, ActionType::Raise});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopRaise(hand, ctx))
        << "Very loose mode players can raise flop with any hand - no assumptions made";
}

// ========================================
// Hands Not Using Hole Cards
// ========================================

TEST_F(FlopRaisePlausibilityTest, HandNotUsingHoleCards_ShouldBeUnplausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    auto hand = createTopPair(false);
    validateHoleCardUsage(hand, false);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Flop,
                                                             {ctx.personalContext.id, ActionType::Raise});
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopRaise(hand, ctx))
        << "Raising flop without using hole cards is unplausible";
}

// ========================================
// Multiway Pot Scenarios
// ========================================

TEST_F(FlopRaisePlausibilityTest, MultiwayWithNoPair_ShouldBeUnplausible)
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
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Flop,
                                                             {ctx.personalContext.id, ActionType::Raise});
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopRaise(hand, ctx))
        << "Raising flop with no pair in multiway pot is unplausible";
}

TEST_F(FlopRaisePlausibilityTest, MultiwayWithWeakPair_ShouldBeUnplausible)
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
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Flop,
                                                             {ctx.personalContext.id, ActionType::Raise});
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopRaise(hand, ctx))
        << "Raising flop with weak pair in multiway pot is unplausible";
}

TEST_F(FlopRaisePlausibilityTest, MultiwayWithTopPair_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createTopPair();
    setDryBoard(hand);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Flop,
                                                             {ctx.personalContext.id, ActionType::Raise});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopRaise(hand, ctx))
        << "Raising flop with top pair in multiway pot is plausible";
}

// ========================================
// Strong Hands Always Plausible
// ========================================

TEST_F(FlopRaisePlausibilityTest, StrongHandsAlwaysPlausible_TwoPair)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createTwoPair();
    setDryBoard(hand);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Flop,
                                                             {ctx.personalContext.id, ActionType::Raise});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopRaise(hand, ctx))
        << "Raising flop with two pair is always plausible";
}

TEST_F(FlopRaisePlausibilityTest, StrongHandsAlwaysPlausible_Set)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createTrips();
    setDryBoard(hand);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Flop,
                                                             {ctx.personalContext.id, ActionType::Raise});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopRaise(hand, ctx))
        << "Raising flop with set is always plausible";
}

TEST_F(FlopRaisePlausibilityTest, StrongHandsAlwaysPlausible_Straight)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createStraight();
    setFlushDrawBoard(hand);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Flop,
                                                             {ctx.personalContext.id, ActionType::Raise});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopRaise(hand, ctx))
        << "Raising flop with straight is always plausible";
}

TEST_F(FlopRaisePlausibilityTest, StrongHandsAlwaysPlausible_Flush)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createFlush();
    setStraightDrawBoard(hand);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Flop,
                                                             {ctx.personalContext.id, ActionType::Raise});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopRaise(hand, ctx))
        << "Raising flop with flush is always plausible";
}

} // namespace pkt::test
