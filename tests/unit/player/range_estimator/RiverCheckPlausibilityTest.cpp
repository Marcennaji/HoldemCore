#include <gtest/gtest.h>
#include "HandPlausibilityTestBase.h"
#include "core/player/range/HandPlausibilityChecker.h"

using namespace pkt::core::player;

namespace pkt::test
{

class RiverCheckPlausibilityTest : public HandPlausibilityTestBase
{
  protected:
    void SetUp() override { HandPlausibilityTestBase::SetUp(); }
};

// ========================================
// Maniac Player Exception Tests
// ========================================

TEST_F(RiverCheckPlausibilityTest, ManiacPlayerAnyHand_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setPlayerStatistics(ctx, 4.0f, 60.0f, 100);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Check});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverCheck(hand, ctx))
        << "Maniac players can check river with any hand - no assumptions made";
}

TEST_F(RiverCheckPlausibilityTest, VeryLooseModeAnyHand_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setManiacProfile(ctx);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Check});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverCheck(hand, ctx))
        << "Very loose mode players can check river with any hand - no assumptions made";
}

// ========================================
// Hands Not Using Hole Cards
// ========================================

TEST_F(RiverCheckPlausibilityTest, HandNotUsingHoleCards_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    auto hand = createTopPair(false);
    validateHoleCardUsage(hand, false);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Check});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverCheck(hand, ctx))
        << "Checking river without using hole cards is plausible";
}

// ========================================
// Multiway Pot Scenarios
// ========================================

TEST_F(RiverCheckPlausibilityTest, MultiwayWithNoPair_ShouldBePlausible)
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
                                                             {ctx.personalContext.id, ActionType::Check});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverCheck(hand, ctx))
        << "Checking river with no pair in multiway pot is plausible";
}

TEST_F(RiverCheckPlausibilityTest, MultiwayWithWeakPair_ShouldBePlausible)
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
                                                             {ctx.personalContext.id, ActionType::Check});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverCheck(hand, ctx))
        << "Checking river with weak pair in multiway pot is plausible";
}

TEST_F(RiverCheckPlausibilityTest, MultiwayWithTopPair_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createTopPair();
    setDryBoard(hand);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Check});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverCheck(hand, ctx))
        << "Checking river with top pair in multiway pot is plausible";
}

// ========================================
// Strong Hands Always Plausible
// ========================================

TEST_F(RiverCheckPlausibilityTest, StrongHandsAlwaysPlausible_TwoPair)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createTwoPair();
    setDryBoard(hand);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Check});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverCheck(hand, ctx))
        << "Checking river with two pair is always plausible";
}

TEST_F(RiverCheckPlausibilityTest, StrongHandsAlwaysPlausible_Set)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createTrips();
    setDryBoard(hand);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Check});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverCheck(hand, ctx))
        << "Checking river with set is always plausible";
}

TEST_F(RiverCheckPlausibilityTest, StrongHandsAlwaysPlausible_Straight)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createStraight();
    setFlushDrawBoard(hand);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Check});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverCheck(hand, ctx))
        << "Checking river with straight is always plausible";
}

TEST_F(RiverCheckPlausibilityTest, StrongHandsAlwaysPlausible_Flush)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createFlush();
    setStraightDrawBoard(hand);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Check});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverCheck(hand, ctx))
        << "Checking river with flush is always plausible";
}

} // namespace pkt::test
