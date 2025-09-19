#include "HandPlausibilityTestBase.h"
#include <memory>
#include "core/player/Player.h"

namespace pkt::test
{

void HandPlausibilityTestBase::SetUp()
{
    // Base setup for all plausibility tests
}

void HandPlausibilityTestBase::TearDown()
{
    // Base cleanup for all plausibility tests
}

// ========================================
// Hand Context Creation Utilities
// ========================================

CurrentHandContext HandPlausibilityTestBase::createHandContext(bool hasPosition, bool isPassive)
{
    CurrentHandContext ctx;

    // Initialize personal context
    ctx.personalContext.hasPosition = hasPosition;
    ctx.personalContext.id = 1;
    ctx.personalContext.cash = 1000;
    ctx.personalContext.totalBetAmount = 0;
    ctx.personalContext.m = 50;

    // Initialize common context
    ctx.commonContext.gameState = GameState::Flop;
    ctx.commonContext.smallBlind = 10;
    ctx.commonContext.stringBoard = "Ah Kh Qs"; // Example board

    // Initialize player list with 2 players by default
    ctx.commonContext.playersContext.actingPlayersList = createMockPlayerList(2);
    ctx.commonContext.playersContext.nbPlayers = 2;

    // Set default betting context
    ctx.commonContext.bettingContext.pot = 100;
    ctx.commonContext.bettingContext.flopBetsOrRaisesNumber = 0;
    ctx.commonContext.bettingContext.turnBetsOrRaisesNumber = 0;
    ctx.commonContext.bettingContext.riverBetsOrRaisesNumber = 0;

    // Set default statistics based on passive/aggressive flag
    if (isPassive)
    {
        setTightPassiveProfile(ctx);
    }
    else
    {
        // Default to moderate aggressive
        setPlayerStatistics(ctx, 2.5f, 40.0f, 100);
    }

    return ctx;
}

CurrentHandContext HandPlausibilityTestBase::createMultiwayContext(int numPlayers, bool hasPosition)
{
    auto ctx = createHandContext(hasPosition, false);
    setMultiPlayerPot(ctx, numPlayers);
    return ctx;
}

CurrentHandContext HandPlausibilityTestBase::createHeadsUpContext(bool hasPosition)
{
    return createMultiwayContext(2, hasPosition);
}

// ========================================
// Player Statistics & Profile Utilities
// ========================================

void HandPlausibilityTestBase::setPlayerStatistics(CurrentHandContext& ctx, float aggressionFactor,
                                                   float aggressionFreq, int handsSample)
{
    // Initialize all statistics structures
    ctx.personalContext.statistics.totalHands = handsSample;

    // Set flop statistics (most relevant for our tests)
    ctx.personalContext.statistics.flopStatistics.hands = handsSample;
    ctx.personalContext.statistics.flopStatistics.bets =
        static_cast<long>(handsSample * aggressionFreq / 100.0f * 0.3f);
    ctx.personalContext.statistics.flopStatistics.raises =
        static_cast<long>(handsSample * aggressionFreq / 100.0f * 0.2f);
    ctx.personalContext.statistics.flopStatistics.calls = static_cast<long>(handsSample * 0.3f);
    ctx.personalContext.statistics.flopStatistics.checks =
        handsSample - ctx.personalContext.statistics.flopStatistics.bets -
        ctx.personalContext.statistics.flopStatistics.raises - ctx.personalContext.statistics.flopStatistics.calls;

    // Set turn statistics
    ctx.personalContext.statistics.turnStatistics.hands = handsSample;
    ctx.personalContext.statistics.turnStatistics.bets =
        static_cast<long>(handsSample * aggressionFreq / 100.0f * 0.25f);
    ctx.personalContext.statistics.turnStatistics.raises =
        static_cast<long>(handsSample * aggressionFreq / 100.0f * 0.15f);
    ctx.personalContext.statistics.turnStatistics.calls = static_cast<long>(handsSample * 0.25f);

    // Set river statistics
    ctx.personalContext.statistics.riverStatistics.hands = handsSample;
    ctx.personalContext.statistics.riverStatistics.bets =
        static_cast<long>(handsSample * aggressionFreq / 100.0f * 0.2f);
    ctx.personalContext.statistics.riverStatistics.raises =
        static_cast<long>(handsSample * aggressionFreq / 100.0f * 0.1f);
    ctx.personalContext.statistics.riverStatistics.calls = static_cast<long>(handsSample * 0.2f);

    // Note: Aggression factor is calculated by the methods, so we set up the raw stats
    // to produce the desired aggression factor and frequency
}

void HandPlausibilityTestBase::setTightPassiveProfile(CurrentHandContext& ctx)
{
    setPlayerStatistics(ctx, PASSIVE_AGGRESSION_FACTOR, PASSIVE_AGGRESSION_FREQUENCY, 100);
}

void HandPlausibilityTestBase::setLooseAggressiveProfile(CurrentHandContext& ctx)
{
    setPlayerStatistics(ctx, AGGRESSIVE_AGGRESSION_FACTOR, AGGRESSIVE_AGGRESSION_FREQUENCY, 100);
}

void HandPlausibilityTestBase::setManiacProfile(CurrentHandContext& ctx)
{
    setPlayerStatistics(ctx, MANIAC_AGGRESSION_FACTOR, MANIAC_AGGRESSION_FREQUENCY, 100);
    ctx.personalContext.actions.isInVeryLooseMode = true;
}

void HandPlausibilityTestBase::setInsufficientStatistics(CurrentHandContext& ctx)
{
    setPlayerStatistics(ctx, 2.0f, 30.0f, 25); // Below MIN_HANDS_STATISTICS_ACCURATE
}

// ========================================
// Game Context & Betting Utilities
// ========================================

void HandPlausibilityTestBase::setMultiPlayerPot(CurrentHandContext& ctx, int numPlayers)
{
    ctx.commonContext.playersContext.actingPlayersList = createMockPlayerList(numPlayers);
    ctx.commonContext.playersContext.nbPlayers = numPlayers;
}

void HandPlausibilityTestBase::setAggressiveAction(CurrentHandContext& ctx, int betsOrRaises)
{
    ctx.commonContext.bettingContext.flopBetsOrRaisesNumber = betsOrRaises;
}

void HandPlausibilityTestBase::setPassiveAction(CurrentHandContext& ctx)
{
    ctx.commonContext.bettingContext.flopBetsOrRaisesNumber = 0;
    ctx.commonContext.bettingContext.turnBetsOrRaisesNumber = 0;
    ctx.commonContext.bettingContext.riverBetsOrRaisesNumber = 0;
}

void HandPlausibilityTestBase::setPreviousAggressor(CurrentHandContext& ctx, GameState street)
{
    switch (street)
    {
    case GameState::Flop:
        ctx.personalContext.actions.flopIsAggressor = true;
        break;
    case GameState::Turn:
        ctx.personalContext.actions.turnIsAggressor = true;
        break;
    case GameState::River:
        ctx.personalContext.actions.riverIsAggressor = true;
        break;
    default:
        ctx.personalContext.actions.preflopIsAggressor = true;
        break;
    }
}

void HandPlausibilityTestBase::setFacingAggression(CurrentHandContext& ctx)
{
    ctx.personalContext.actions.preflopIsAggressor = false;
    ctx.personalContext.actions.flopIsAggressor = false;
    ctx.personalContext.actions.turnIsAggressor = false;
    ctx.personalContext.actions.riverIsAggressor = false;
}

// ========================================
// Hand Analysis Creation Utilities
// ========================================

PostFlopAnalysisFlags HandPlausibilityTestBase::createHandAnalysis()
{
    PostFlopAnalysisFlags hand = {};
    // All fields initialized to false by default
    return hand;
}

PostFlopAnalysisFlags HandPlausibilityTestBase::createTopPair(bool usesHoleCards)
{
    auto hand = createHandAnalysis();
    hand.usesFirst = usesHoleCards;
    hand.usesSecond = usesHoleCards;
    hand.isOnePair = true;
    hand.isTopPair = true;
    hand.isNoPair = false;
    return hand;
}

PostFlopAnalysisFlags HandPlausibilityTestBase::createOverPair(bool usesHoleCards)
{
    auto hand = createHandAnalysis();
    hand.usesFirst = usesHoleCards;
    hand.usesSecond = usesHoleCards;
    hand.isOnePair = true;
    hand.isOverPair = true;
    hand.isPocketPair = true;
    hand.isNoPair = false;
    return hand;
}

PostFlopAnalysisFlags HandPlausibilityTestBase::createMiddlePair(bool usesHoleCards)
{
    auto hand = createHandAnalysis();
    hand.usesFirst = usesHoleCards;
    hand.usesSecond = usesHoleCards;
    hand.isOnePair = true;
    hand.isMiddlePair = true;
    hand.isNoPair = false;
    return hand;
}

PostFlopAnalysisFlags HandPlausibilityTestBase::createTwoPair(bool usesHoleCards)
{
    auto hand = createHandAnalysis();
    hand.usesFirst = usesHoleCards;
    hand.usesSecond = usesHoleCards;
    hand.isTwoPair = true;
    hand.isNoPair = false;
    hand.isOnePair = false;
    return hand;
}

PostFlopAnalysisFlags HandPlausibilityTestBase::createTrips(bool usesHoleCards)
{
    auto hand = createHandAnalysis();
    hand.usesFirst = usesHoleCards;
    hand.usesSecond = usesHoleCards;
    hand.isTrips = true;
    hand.isNoPair = false;
    hand.isOnePair = false;
    return hand;
}

PostFlopAnalysisFlags HandPlausibilityTestBase::createStraight(bool usesHoleCards)
{
    auto hand = createHandAnalysis();
    hand.usesFirst = usesHoleCards;
    hand.usesSecond = usesHoleCards;
    hand.isStraight = true;
    hand.isNoPair = false;
    return hand;
}

PostFlopAnalysisFlags HandPlausibilityTestBase::createFlush(bool usesHoleCards)
{
    auto hand = createHandAnalysis();
    hand.usesFirst = usesHoleCards;
    hand.usesSecond = usesHoleCards;
    hand.isFlush = true;
    hand.isNoPair = false;
    return hand;
}

PostFlopAnalysisFlags HandPlausibilityTestBase::createFullHouse(bool usesHoleCards)
{
    auto hand = createHandAnalysis();
    hand.usesFirst = usesHoleCards;
    hand.usesSecond = usesHoleCards;
    hand.isFullHouse = true;
    hand.isNoPair = false;
    return hand;
}

PostFlopAnalysisFlags HandPlausibilityTestBase::createQuads(bool usesHoleCards)
{
    auto hand = createHandAnalysis();
    hand.usesFirst = usesHoleCards;
    hand.usesSecond = usesHoleCards;
    hand.isQuads = true;
    hand.isNoPair = false;
    return hand;
}

// ========================================
// Draw and Weak Hand Utilities
// ========================================

PostFlopAnalysisFlags HandPlausibilityTestBase::createOvercards(bool usesHoleCards)
{
    auto hand = createHandAnalysis();
    hand.usesFirst = usesHoleCards;
    hand.usesSecond = usesHoleCards;
    hand.isOverCards = true;
    hand.isNoPair = true;
    return hand;
}

PostFlopAnalysisFlags HandPlausibilityTestBase::createFlushDraw(int outs, bool usesHoleCards)
{
    auto hand = createHandAnalysis();
    hand.usesFirst = usesHoleCards;
    hand.usesSecond = usesHoleCards;
    hand.flushOuts = outs;
    hand.is4Flush = (outs == 9); // Standard flush draw
    hand.isNoPair = true;        // Assuming draw without pair
    return hand;
}

PostFlopAnalysisFlags HandPlausibilityTestBase::createStraightDraw(int outs, bool usesHoleCards)
{
    auto hand = createHandAnalysis();
    hand.usesFirst = usesHoleCards;
    hand.usesSecond = usesHoleCards;
    hand.straightOuts = outs;
    hand.isNoPair = true; // Assuming draw without pair
    return hand;
}

PostFlopAnalysisFlags HandPlausibilityTestBase::createComboDraw(int flushOuts, int straightOuts, bool usesHoleCards)
{
    auto hand = createHandAnalysis();
    hand.usesFirst = usesHoleCards;
    hand.usesSecond = usesHoleCards;
    hand.flushOuts = flushOuts;
    hand.straightOuts = straightOuts;
    hand.is4Flush = (flushOuts >= 9);
    hand.isNoPair = true; // Assuming draw without pair
    return hand;
}

// ========================================
// Board Texture Utilities
// ========================================

void HandPlausibilityTestBase::setDrawHeavyBoard(PostFlopAnalysisFlags& hand)
{
    hand.isFlushDrawPossible = true;
    hand.isStraightDrawPossible = true;
    hand.isFullHousePossible = false; // Not paired
}

void HandPlausibilityTestBase::setDryBoard(PostFlopAnalysisFlags& hand)
{
    hand.isFlushDrawPossible = false;
    hand.isStraightDrawPossible = false;
    hand.isFullHousePossible = false;
}

void HandPlausibilityTestBase::setPairedBoard(PostFlopAnalysisFlags& hand)
{
    hand.isFullHousePossible = true;
}

void HandPlausibilityTestBase::setRainbowBoard(PostFlopAnalysisFlags& hand)
{
    hand.isFlushDrawPossible = false;
    hand.isFlushPossible = false;
    hand.is3Flush = false;
    hand.is4Flush = false;
}

void HandPlausibilityTestBase::setFlushDrawBoard(PostFlopAnalysisFlags& hand)
{
    hand.isFlushDrawPossible = true;
    hand.isFlushPossible = true;
}

void HandPlausibilityTestBase::setStraightDrawBoard(PostFlopAnalysisFlags& hand)
{
    hand.isStraightDrawPossible = true;
    hand.isStraightPossible = true;
}

// ========================================
// Validation Utilities
// ========================================

void HandPlausibilityTestBase::validateHoleCardUsage(const PostFlopAnalysisFlags& hand, bool shouldUseCards)
{
    EXPECT_EQ(hand.usesFirst || hand.usesSecond, shouldUseCards)
        << "Hand should " << (shouldUseCards ? "" : "not ") << "use hole cards";
}

void HandPlausibilityTestBase::validateStatisticalReliability(const CurrentHandContext& ctx, bool shouldBeReliable)
{
    bool isReliable = ctx.personalContext.statistics.flopStatistics.hands >= MIN_HANDS_STATISTICS_ACCURATE;
    EXPECT_EQ(isReliable, shouldBeReliable)
        << "Statistics should " << (shouldBeReliable ? "" : "not ") << "be reliable for making assumptions";
}

// ========================================
// Private Helper Methods
// ========================================

std::shared_ptr<std::list<std::shared_ptr<Player>>> HandPlausibilityTestBase::createMockPlayerList(int numPlayers)
{
    auto playerList = std::make_shared<std::list<std::shared_ptr<Player>>>();

    // Create mock players for testing
    for (int i = 0; i < numPlayers; ++i)
    {
        // Note: This creates a basic structure for testing
        // In a real implementation, you might need to create actual Player objects
        // or use mocks depending on your testing framework
        auto player = std::shared_ptr<Player>(nullptr); // Placeholder
        playerList->push_back(player);
    }

    return playerList;
}
}