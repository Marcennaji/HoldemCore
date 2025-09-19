#include <gtest/gtest.h>
#include "HandPlausibilityTestBase.h"
#include "core/player/range/HandPlausibilityChecker.h"

using namespace pkt::core::player;

namespace pkt::test
{

class TurnAllinPlausibilityTest : public HandPlausibilityTestBase
{
  protected:
    void SetUp() override { HandPlausibilityTestBase::SetUp(); }
};

// ========================================
// Maniac Player Exception Tests
// ========================================

TEST_F(TurnAllinPlausibilityTest, ManiacPlayerAnyHand_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setPlayerStatistics(ctx, 4.0f, 60.0f, 100);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnAllin(hand, ctx))
        << "Maniac players can go all-in with any hand - no assumptions made";
}

TEST_F(TurnAllinPlausibilityTest, VeryLooseModeAnyHand_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setManiacProfile(ctx);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnAllin(hand, ctx))
        << "Very loose mode players can go all-in with any hand - no assumptions made";
}

// ========================================
// Hands Not Using Hole Cards
// ========================================

TEST_F(TurnAllinPlausibilityTest, HandNotUsingHoleCards_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    auto hand = createTopPair(false);
    validateHoleCardUsage(hand, false);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnAllin(hand, ctx))
        << "All-in without using hole cards is plausible";
}

// ========================================
// Double Raise Scenario
// ========================================

TEST_F(TurnAllinPlausibilityTest, DoubleRaiseWithWeakHand_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    ctx.personalContext.actions.currentHandActions.reset();
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Turn,
                                                             {ctx.personalContext.id, ActionType::Raise});
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Turn,
                                                             {ctx.personalContext.id, ActionType::Raise});
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isNoPair = true;
    hand.isOverCards = true;
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnAllin(hand, ctx))
        << "Double raise all-in with weak hand is plausible";
}

TEST_F(TurnAllinPlausibilityTest, DoubleRaiseWithStraight_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    ctx.personalContext.actions.currentHandActions.reset();
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Turn,
                                                             {ctx.personalContext.id, ActionType::Raise});
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Turn,
                                                             {ctx.personalContext.id, ActionType::Raise});
    auto hand = createStraight();
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnAllin(hand, ctx))
        << "Double raise all-in with straight is plausible";
}

// ========================================
// Triple Raise Scenario
// ========================================

TEST_F(TurnAllinPlausibilityTest, TripleRaiseWithSet_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    ctx.personalContext.actions.currentHandActions.reset();
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Turn,
                                                             {ctx.personalContext.id, ActionType::Raise});
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Turn,
                                                             {ctx.personalContext.id, ActionType::Raise});
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Turn,
                                                             {ctx.personalContext.id, ActionType::Raise});
    auto hand = createTrips();
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnAllin(hand, ctx))
        << "Triple raise all-in with set is plausible";
}

TEST_F(TurnAllinPlausibilityTest, TripleRaiseWithFlush_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    ctx.personalContext.actions.currentHandActions.reset();
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Turn,
                                                             {ctx.personalContext.id, ActionType::Raise});
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Turn,
                                                             {ctx.personalContext.id, ActionType::Raise});
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Turn,
                                                             {ctx.personalContext.id, ActionType::Raise});
    auto hand = createFlush();
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnAllin(hand, ctx))
        << "Triple raise all-in with flush is plausible";
}

} // namespace pkt::test
