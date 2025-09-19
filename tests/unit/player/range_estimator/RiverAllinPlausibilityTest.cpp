#include <gtest/gtest.h>
#include "HandPlausibilityTestBase.h"
#include "core/player/range/HandPlausibilityChecker.h"

using namespace pkt::core::player;

namespace pkt::test
{

class RiverAllinPlausibilityTest : public HandPlausibilityTestBase
{
  protected:
    void SetUp() override { HandPlausibilityTestBase::SetUp(); }
};

// ========================================
// Maniac Player Exception Tests
// ========================================

TEST_F(RiverAllinPlausibilityTest, ManiacPlayerAnyHand_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setPlayerStatistics(ctx, 4.0f, 60.0f, 100);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Allin});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverAllin(hand, ctx))
        << "Maniac players can go all-in on river with any hand - no assumptions made";
}

TEST_F(RiverAllinPlausibilityTest, VeryLooseModeAnyHand_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setManiacProfile(ctx);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Allin});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverAllin(hand, ctx))
        << "Very loose mode players can go all-in on river with any hand - no assumptions made";
}

// ========================================
// Hands Not Using Hole Cards
// ========================================

TEST_F(RiverAllinPlausibilityTest, DISABLED_HandNotUsingHoleCards_ShouldBeUnplausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    auto hand = createTopPair(false);
    validateHoleCardUsage(hand, false);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Allin});
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenRiverAllin(hand, ctx))
        << "Going all-in on river without using hole cards is unplausible";
}

// ========================================
// Multiway Pot Scenarios
// ========================================

TEST_F(RiverAllinPlausibilityTest, DISABLED_MultiwayWithNoPair_ShouldBeUnplausible)
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
                                                             {ctx.personalContext.id, ActionType::Allin});
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenRiverAllin(hand, ctx))
        << "Going all-in on river with no pair in multiway pot is unplausible";
}

TEST_F(RiverAllinPlausibilityTest, DISABLED_MultiwayWithWeakPair_ShouldBeUnplausible)
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
                                                             {ctx.personalContext.id, ActionType::Allin});
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenRiverAllin(hand, ctx))
        << "Going all-in on river with weak pair in multiway pot is unplausible";
}

TEST_F(RiverAllinPlausibilityTest, MultiwayWithTopPair_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createTopPair();
    setDryBoard(hand);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Allin});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverAllin(hand, ctx))
        << "Going all-in on river with top pair in multiway pot is plausible";
}

// ========================================
// Strong Hands Always Plausible
// ========================================

TEST_F(RiverAllinPlausibilityTest, StrongHandsAlwaysPlausible_TwoPair)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createTwoPair();
    setDryBoard(hand);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Allin});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverAllin(hand, ctx))
        << "Going all-in on river with two pair is always plausible";
}

TEST_F(RiverAllinPlausibilityTest, StrongHandsAlwaysPlausible_Set)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createTrips();
    setDryBoard(hand);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Allin});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverAllin(hand, ctx))
        << "Going all-in on river with set is always plausible";
}

TEST_F(RiverAllinPlausibilityTest, StrongHandsAlwaysPlausible_Straight)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createStraight();
    setFlushDrawBoard(hand);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Allin});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverAllin(hand, ctx))
        << "Going all-in on river with straight is always plausible";
}

TEST_F(RiverAllinPlausibilityTest, StrongHandsAlwaysPlausible_Flush)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 4);
    auto hand = createFlush();
    setStraightDrawBoard(hand);
    ctx.personalContext.actions.currentHandActions.addAction(GameState::River,
                                                             {ctx.personalContext.id, ActionType::Allin});
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenRiverAllin(hand, ctx))
        << "Going all-in on river with flush is always plausible";
}

} // namespace pkt::test
