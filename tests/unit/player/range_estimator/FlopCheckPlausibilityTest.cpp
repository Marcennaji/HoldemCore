#include <gtest/gtest.h>
#include "HandPlausibilityTestBase.h"
#include "core/player/range/HandPlausibilityChecker.h"

using namespace pkt::core::player;

namespace pkt::test
{

class FlopCheckPlausibilityTest : public HandPlausibilityTestBase
{
  protected:
    void SetUp() override { HandPlausibilityTestBase::SetUp(); }
};

// ========================================
// Position-Based Slowplay Logic Tests
// ========================================

TEST_F(FlopCheckPlausibilityTest, InPositionStrongHandOnDangerousBoard_ShouldBeUnplausible)
{
    // Arrange: Player in position, not passive, strong hand on dangerous board
    auto ctx = createHandContext(true, false); // hasPosition=true, isPassive=false
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 3); // 3 players

    // Top pair on flush + straight draw board
    auto hand = createTopPair();
    setDrawHeavyBoard(hand);

    // Act & Assert
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(hand, ctx))
        << "Should not slowplay top pair on dangerous board in position";
}

TEST_F(FlopCheckPlausibilityTest, InPositionOverPairOnDangerousBoard_ShouldBeUnplausible)
{
    // Arrange: Player in position with overpair on dangerous board
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);
    setMultiPlayerPot(ctx, 2);

    auto hand = createOverPair();
    setDrawHeavyBoard(hand);

    // Act & Assert
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(hand, ctx))
        << "Should not slowplay overpair on dangerous board";
}

TEST_F(FlopCheckPlausibilityTest, InPositionTwoPairOnFlushDrawBoard_ShouldBeUnplausible)
{
    // Arrange: Two pair on flush draw board
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);

    auto hand = createTwoPair();
    setFlushDrawBoard(hand);

    // Act & Assert
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(hand, ctx))
        << "Should not slowplay two pair on flush draw board";
}

// ========================================
// Strong Hands That Shouldn't Be Slowplayed
// ========================================

TEST_F(FlopCheckPlausibilityTest, InPositionSetOnFlushDrawBoard_ShouldBeUnplausible)
{
    // Arrange: Set on non-paired board with flush draw
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);

    auto hand = createTrips();
    setFlushDrawBoard(hand);
    hand.isFullHousePossible = false; // non-paired board

    // Act & Assert
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(hand, ctx))
        << "Should not slowplay set on flush draw board";
}

TEST_F(FlopCheckPlausibilityTest, InPositionStraightOnFlushDrawBoard_ShouldBeUnplausible)
{
    // Arrange: Made straight on flush draw board
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);

    auto hand = createStraight();
    setFlushDrawBoard(hand);
    hand.isFullHousePossible = false;

    // Act & Assert
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(hand, ctx))
        << "Should not slowplay straight on flush draw board";
}

// ========================================
// Multiway Pot Scenarios
// ========================================

TEST_F(FlopCheckPlausibilityTest, InPositionTopPairMultiwayPot_ShouldBeUnplausible)
{
    // Arrange: Top pair in 3+ player pot
    auto ctx = createHandContext(true, false);
    setPlayerStatistics(ctx, 2.0f, 30.0f, 100);
    setMultiPlayerPot(ctx, 4); // 4 players

    auto hand = createTopPair();
    hand.isFullHousePossible = false;

    // Act & Assert
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(hand, ctx))
        << "Should not check top pair in multiway pot in position";
}

TEST_F(FlopCheckPlausibilityTest, InPositionSetMultiwayPot_ShouldBeUnplausible)
{
    // Arrange: Set in multiway pot
    auto ctx = createHandContext(true, false);
    setPlayerStatistics(ctx, 2.5f, 35.0f, 100);
    setMultiPlayerPot(ctx, 3);

    auto hand = createTrips();
    hand.isFullHousePossible = false;

    // Act & Assert
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(hand, ctx))
        << "Should not check set in multiway pot";
}

// ========================================
// Paired Board Scenarios
// ========================================

TEST_F(FlopCheckPlausibilityTest, InPositionOverPairOnPairedBoard_ShouldBeUnplausible)
{
    // Arrange: Pocket overpair on paired board
    auto ctx = createHandContext(true, false);
    setPlayerStatistics(ctx, 2.2f, 30.0f, 100);

    auto hand = createOverPair();
    setPairedBoard(hand);

    // Act & Assert
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(hand, ctx))
        << "Should not check overpair on paired board";
}

// ========================================
// Passive Player Exceptions
// ========================================

TEST_F(FlopCheckPlausibilityTest, PassivePlayerStrongHand_ShouldBePlausible)
{
    // Arrange: Passive player statistics
    auto ctx = createHandContext(true, true); // hasPosition=true, isPassive=true
    setTightPassiveProfile(ctx);

    auto hand = createTopPair();
    setDrawHeavyBoard(hand);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(hand, ctx))
        << "Passive player can slowplay strong hands";
}

// ========================================
// Out-of-Position Scenarios
// ========================================

TEST_F(FlopCheckPlausibilityTest, OutOfPositionStrongHand_ShouldBePlausible)
{
    // Arrange: Out of position
    auto ctx = createHandContext(false, false); // hasPosition=false
    setLooseAggressiveProfile(ctx);

    auto hand = createTopPair();
    setDrawHeavyBoard(hand);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(hand, ctx))
        << "Out of position players can check strong hands";
}

// ========================================
// Hands Not Using Hole Cards
// ========================================

TEST_F(FlopCheckPlausibilityTest, HandNotUsingHoleCards_ShouldBePlausible)
{
    // Arrange: Hand doesn't use hole cards
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);

    auto hand = createTopPair(false); // doesn't use hole cards
    validateHoleCardUsage(hand, false);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(hand, ctx))
        << "Hands not using hole cards can be checked";
}

// ========================================
// Premium Hands That Can Be Slowplayed
// ========================================

TEST_F(FlopCheckPlausibilityTest, InPositionFullHouseCanSlowplay_ShouldBePlausible)
{
    // Arrange: Full house (nuts)
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);

    auto hand = createFullHouse();
    setDrawHeavyBoard(hand);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(hand, ctx))
        << "Full house can be slowplayed even on dangerous boards";
}

TEST_F(FlopCheckPlausibilityTest, InPositionQuadsCanSlowplay_ShouldBePlausible)
{
    // Arrange: Quads (nuts)
    auto ctx = createHandContext(true, false);
    setLooseAggressiveProfile(ctx);

    auto hand = createQuads();
    setFlushDrawBoard(hand);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(hand, ctx))
        << "Quads can always be slowplayed";
}

// ========================================
// Insufficient Statistics
// ========================================

TEST_F(FlopCheckPlausibilityTest, InsufficientStatistics_ShouldBeUnplausible)
{
    // Arrange: Not enough hands for reliable statistics
    auto ctx = createHandContext(true, false);
    setInsufficientStatistics(ctx);
    validateStatisticalReliability(ctx, false);

    auto hand = createTopPair();
    setDrawHeavyBoard(hand);

    // Act & Assert
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(hand, ctx))
        << "Rational behavior assumed with insufficient statistics - should not slowplay top pair on dangerous board";
}

// ========================================
// Heads-Up Scenarios
// ========================================

TEST_F(FlopCheckPlausibilityTest, HeadsUpMiddlePairDangerousBoard_ShouldBePlausible)
{
    // Arrange: Heads-up, middle pair on dangerous board
    auto ctx = createHeadsUpContext(true);
    setTightPassiveProfile(ctx); // Passive players can check middle pair on dangerous board

    auto hand = createMiddlePair();
    setDrawHeavyBoard(hand);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(hand, ctx))
        << "Middle pair can be checked heads-up even on dangerous board";
}

// ========================================
// Dry Board Scenarios
// ========================================

TEST_F(FlopCheckPlausibilityTest, InPositionTopPairDryBoard_ShouldBePlausible)
{
    // Arrange: Top pair on dry board (no draws)
    auto ctx = createHandContext(true, false);
    setTightPassiveProfile(ctx); // Passive players can slowplay top pair on dry board
    setMultiPlayerPot(ctx, 3);

    auto hand = createTopPair();
    setDryBoard(hand);

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(hand, ctx))
        << "Top pair can be slowplayed on dry board";
}

// ========================================
// Maniac Player Profile
// ========================================

TEST_F(FlopCheckPlausibilityTest, ManiacPlayerStrongHand_ShouldBeUnplausible)
{
    // Arrange: Very aggressive (maniac) player
    auto ctx = createHandContext(true, false);
    setManiacProfile(ctx);

    auto hand = createTopPair();
    setDrawHeavyBoard(hand);

    // Act & Assert
    EXPECT_TRUE(HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(hand, ctx))
        << "Maniac players won't slowplay strong hands on a draw-heavy flop board";
}

// ========================================
// Edge Cases
// ========================================

TEST_F(FlopCheckPlausibilityTest, MiddlePairHeadsUpNonDangerousBoard_ShouldBePlausible)
{
    // Arrange: Medium strength hand in favorable conditions
    auto ctx = createHeadsUpContext(true);
    setPlayerStatistics(ctx, 2.8f, 40.0f, 100);

    auto hand = createMiddlePair();
    hand.isFullHousePossible = false;
    // Not setting dangerous board flags - relatively safe

    // Act & Assert
    EXPECT_FALSE(HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(hand, ctx))
        << "Middle pair can be checked heads-up on safe board";
}
} // namespace pkt::test