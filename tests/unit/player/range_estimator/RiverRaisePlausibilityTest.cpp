#include <gtest/gtest.h>
#include "HandPlausibilityTestBase.h"
#include "core/player/range/HandPlausibilityChecker.h"

using namespace pkt::core::player;

namespace pkt::test
{

class RiverRaisePlausibilityTest : public HandPlausibilityTestBase
{
  protected:
    void SetUp() override { HandPlausibilityTestBase::SetUp(); }
};

// ========================================
// Maniac Player Exception Tests
// ========================================

TEST_F(RiverRaisePlausibilityTest, ManiacPlayerAnyHand_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setPlayerStatistics(ctx, 4.0f, 60.0f, 100);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Raise});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverRaise(hand, ctx))
        << "Maniac players can raise river with any hand - no assumptions made";
}

TEST_F(RiverRaisePlausibilityTest, VeryLooseModeAnyHand_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setManiacProfile(ctx);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Raise});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverRaise(hand, ctx))
        << "Very loose mode players can raise river with any hand - no assumptions made";
}

// ========================================
// Hands Not Using Hole Cards
// ========================================

TEST_F(RiverRaisePlausibilityTest, HandNotUsingHoleCards_ShouldBeUnplausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    auto hand = createTopPair(false);
    validateHoleCardUsage(hand, false);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Raise});
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenRiverRaise(hand, ctx))
        << "Raising river without using hole cards is unplausible";
}

// ========================================
// Multiway Pot Scenarios
// ========================================

TEST_F(RiverRaisePlausibilityTest, MultiwayWithNoPair_ShouldBeUnplausible)
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
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Raise});
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenRiverRaise(hand, ctx))
        << "Raising river with no pair in multiway pot is unplausible";
}

TEST_F(RiverRaisePlausibilityTest, MultiwayWithWeakPair_ShouldBeUnplausible)
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
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Raise});
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenRiverRaise(hand, ctx))
        << "Raising river with weak pair in multiway pot is unplausible";
}

TEST_F(RiverRaisePlausibilityTest, MultiwayWithTopPair_ShouldBeUnplausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createTopPair();
    setDryBoard(hand);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Raise});
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenRiverRaise(hand, ctx))
        << "Raising river with top pair in multiway pot is not plausible";
}

// ========================================
// Strong Hands Always Plausible
// ========================================

TEST_F(RiverRaisePlausibilityTest, StrongHandsAlwaysPlausible_TwoPair)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createTwoPair();
    setDryBoard(hand);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Raise});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverRaise(hand, ctx))
        << "Raising river with two pair is always plausible";
}

TEST_F(RiverRaisePlausibilityTest, StrongHandsAlwaysPlausible_Set)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createTrips();
    setDryBoard(hand);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Raise});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverRaise(hand, ctx))
        << "Raising river with set is always plausible";
}

TEST_F(RiverRaisePlausibilityTest, StrongHandsAlwaysPlausible_Straight)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createStraight();
    setFlushDrawBoard(hand);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Raise});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverRaise(hand, ctx))
        << "Raising river with straight is always plausible";
}

TEST_F(RiverRaisePlausibilityTest, StrongHandsAlwaysPlausible_Flush)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createFlush();
    setStraightDrawBoard(hand);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Raise});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverRaise(hand, ctx))
        << "Raising river with flush is always plausible";
}

} // namespace pkt::test
