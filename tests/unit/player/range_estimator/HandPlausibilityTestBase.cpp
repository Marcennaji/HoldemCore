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

    // Calculate raw statistics to achieve desired aggression metrics
    // Aggression Factor = (raises + bets) / calls
    // Aggression Frequency = (raises + bets) / (raises + bets + calls + checks + folds) * 100

    long totalActions = handsSample;
    long folds = static_cast<long>(handsSample * 0.1f); // 10% folds

    // Calculate aggressive actions from frequency
    long aggressiveActions = static_cast<long>(totalActions * aggressionFreq / 100.0f);
    long bets = static_cast<long>(aggressiveActions * 0.6f); // 60% of aggressive actions are bets
    long raises = aggressiveActions - bets;                  // Rest are raises

    // Calculate calls to achieve desired aggression factor
    // aggressionFactor = (raises + bets) / calls
    // calls = (raises + bets) / aggressionFactor
    long calls = (aggressionFactor > 0) ? static_cast<long>((raises + bets) / aggressionFactor)
                                        : static_cast<long>(handsSample * 0.3f); // fallback to 30%

    // Remaining actions are checks
    long checks = totalActions - bets - raises - calls - folds;
    if (checks < 0)
    {
        // Adjust if we over-allocated
        checks = static_cast<long>(handsSample * 0.2f);
        calls = totalActions - bets - raises - checks - folds;
    }

    // Set flop statistics (most relevant for our tests)
    ctx.personalContext.statistics.flopStatistics.hands = handsSample;
    ctx.personalContext.statistics.flopStatistics.bets = bets;
    ctx.personalContext.statistics.flopStatistics.raises = raises;
    ctx.personalContext.statistics.flopStatistics.calls = calls;
    ctx.personalContext.statistics.flopStatistics.checks = checks;
    ctx.personalContext.statistics.flopStatistics.folds = folds;
    ctx.personalContext.statistics.flopStatistics.threeBets = 0;
    ctx.personalContext.statistics.flopStatistics.fourBets = 0;
    ctx.personalContext.statistics.flopStatistics.continuationBets = 0;
    ctx.personalContext.statistics.flopStatistics.continuationBetsOpportunities = 0;

    // Set turn statistics (similar distribution)
    ctx.personalContext.statistics.turnStatistics.hands = handsSample;
    ctx.personalContext.statistics.turnStatistics.bets = bets;
    ctx.personalContext.statistics.turnStatistics.raises = raises;
    ctx.personalContext.statistics.turnStatistics.calls = calls;
    ctx.personalContext.statistics.turnStatistics.checks = checks;
    ctx.personalContext.statistics.turnStatistics.folds = folds;
    ctx.personalContext.statistics.turnStatistics.threeBets = 0;
    ctx.personalContext.statistics.turnStatistics.fourBets = 0;

    // Set river statistics (similar distribution)
    ctx.personalContext.statistics.riverStatistics.hands = handsSample;
    ctx.personalContext.statistics.riverStatistics.bets = bets;
    ctx.personalContext.statistics.riverStatistics.raises = raises;
    ctx.personalContext.statistics.riverStatistics.calls = calls;
    ctx.personalContext.statistics.riverStatistics.checks = checks;
    ctx.personalContext.statistics.riverStatistics.folds = folds;
    ctx.personalContext.statistics.riverStatistics.threeBets = 0;
    ctx.personalContext.statistics.riverStatistics.fourBets = 0;
}

void HandPlausibilityTestBase::setTightPassiveProfile(CurrentHandContext& ctx)
{
    setPlayerStatistics(ctx, PASSIVE_AGGRESSION_FACTOR, PASSIVE_AGGRESSION_FREQUENCY, 100);
}

void HandPlausibilityTestBase::setLooseAggressiveProfile(CurrentHandContext& ctx)
{
    setPlayerStatistics(ctx, AGGRESSIVE_AGGRESSION_FACTOR, AGGRESSIVE_AGGRESSION_FREQUENCY, 100);
}

void HandPlausibilityTestBase::setModerateProfile(CurrentHandContext& ctx)
{
    setPlayerStatistics(ctx, MODERATE_AGGRESSION_FACTOR, MODERATE_AGGRESSION_FREQUENCY, 100);
}

void HandPlausibilityTestBase::setManiacProfile(CurrentHandContext& ctx)
{
    setPlayerStatistics(ctx, MANIAC_AGGRESSION_FACTOR, MANIAC_AGGRESSION_FREQUENCY, 100);
    ctx.personalContext.actions.isInVeryLooseMode = true;
}

void HandPlausibilityTestBase::setInsufficientStatistics(CurrentHandContext& ctx)
{
    setPlayerStatistics(ctx, 2.0f, 30.0f,
                        MIN_HANDS_STATISTICS_ACCURATE - 5); // Below MIN_HANDS_STATISTICS_ACCURATE (30)
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
} // namespace pkt::test