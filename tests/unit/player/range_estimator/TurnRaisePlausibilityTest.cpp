#include <gtest/gtest.h>
#include "HandPlausibilityTestBase.h"
#include "core/player/range/HandPlausibilityChecker.h"

using namespace pkt::core::player;

namespace pkt::test
{

class TurnRaisePlausibilityTest : public HandPlausibilityTestBase
{
  protected:
    void SetUp() override { HandPlausibilityTestBase::SetUp(); }
};

// ========================================
// Maniac Player Exception Tests
// ========================================

TEST_F(TurnRaisePlausibilityTest, ManiacPlayerAnyHand_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setPlayerStatistics(ctx, 4.0f, 60.0f, 100);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnRaise(hand, ctx))
        << "Maniac players can raise with any hand - no assumptions made";
}

TEST_F(TurnRaisePlausibilityTest, VeryLooseModeAnyHand_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setManiacProfile(ctx);
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isOverCards = true;
    hand.isNoPair = true;
    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnRaise(hand, ctx))
        << "Very loose mode players can raise with any hand - no assumptions made";
}

// ========================================
// Hands Not Using Hole Cards
// ========================================

TEST_F(TurnRaisePlausibilityTest, HandNotUsingHoleCards_ShouldBeUnplausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    auto hand = createTopPair(false);
    validateHoleCardUsage(hand, false);
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenTurnRaise(hand, ctx))
        << "Raising without using hole cards is unplausible";
}

// ========================================
// No Flop Action Scenario
// ========================================

TEST_F(TurnRaisePlausibilityTest, NoFlopActionWithWeakHand_ShouldBeUnplausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    ctx.commonContext.bettingContext.flopBetsOrRaisesNumber = 0;
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isNoPair = true;
    hand.isOverCards = true;
    setDryBoard(hand);
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenTurnRaise(hand, ctx))
        << "Raising with weak hand and no flop action is unplausible";
}

TEST_F(TurnRaisePlausibilityTest, NoFlopActionWithTopPair_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    ctx.commonContext.bettingContext.flopBetsOrRaisesNumber = 0;
    auto hand = createTopPair();
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnRaise(hand, ctx))
        << "Raising with top pair and no flop action is plausible";
}

// ========================================
// Turn Action, Not Aggressor Scenario
// ========================================

TEST_F(TurnRaisePlausibilityTest, TurnActionNotAggressorWithWeakHand_ShouldBeUnplausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    ctx.commonContext.bettingContext.flopBetsOrRaisesNumber = 1;
    ctx.personalContext.actions.flopIsAggressor = false;
    auto hand = createHandAnalysis();
    hand.usesFirst = true;
    hand.usesSecond = true;
    hand.isNoPair = true;
    hand.isOverCards = true;
    setDryBoard(hand);
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenTurnRaise(hand, ctx))
        << "Raising with weak hand after flop action, not aggressor, is unplausible";
}

TEST_F(TurnRaisePlausibilityTest, TurnActionNotAggressorWithTopPair_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    ctx.commonContext.bettingContext.flopBetsOrRaisesNumber = 1;
    ctx.personalContext.actions.flopIsAggressor = false;
    auto hand = createTopPair();
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnRaise(hand, ctx))
        << "Raising with top pair after flop action, not aggressor, is plausible";
}

// ========================================
// Multiple Raises Scenario
// ========================================

TEST_F(TurnRaisePlausibilityTest, DoubleRaiseWithWeakHand_ShouldBeUnplausible)
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
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenTurnRaise(hand, ctx))
        << "Double raise with weak hand is unplausible";
}

TEST_F(TurnRaisePlausibilityTest, DoubleRaiseWithTwoPair_ShouldBePlausible)
{
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    ctx.personalContext.actions.currentHandActions.reset();
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Turn,
                                                             {ctx.personalContext.id, ActionType::Raise});
    ctx.personalContext.actions.currentHandActions.addAction(GameState::Turn,
                                                             {ctx.personalContext.id, ActionType::Raise});
    auto hand = createTwoPair();
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnRaise(hand, ctx))
        << "Double raise with two pair is plausible";
}

// ========================================
// Triple Raise Scenario
// ========================================

TEST_F(TurnRaisePlausibilityTest, TripleRaiseWithSet_ShouldBePlausible)
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
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnRaise(hand, ctx))
        << "Triple raise with set is plausible";
}

TEST_F(TurnRaisePlausibilityTest, TripleRaiseWithStraight_ShouldBePlausible)
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
    auto hand = createStraight();
    setDryBoard(hand);
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenTurnRaise(hand, ctx))
        << "Triple raise with straight is plausible";
}

} // namespace pkt::test
