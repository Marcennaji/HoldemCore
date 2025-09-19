#include "PlausibilityHelpers.h"
#include "core/player/range/RangeEstimator.h"
#include "core/player/typedefs.h"

namespace pkt::core::player
{

// Player behavior pattern helpers

bool PlausibilityHelpers::isManiacPlayer(const CurrentHandContext& ctx)
{
    // Check if player is in very loose mode (temporary aggressive behavior)
    if (ctx.personalContext.actions.isInVeryLooseMode)
    {
        return true;
    }

    // Check if player has historically aggressive statistics on flop
    const auto& flop = ctx.personalContext.statistics.flopStatistics;
    return (flop.getAgressionFactor() > 3 && flop.getAgressionFrequency() > 50 &&
            flop.hands > MIN_HANDS_STATISTICS_ACCURATE);
}

bool PlausibilityHelpers::isInPosition(const CurrentHandContext& ctx)
{
    return ctx.personalContext.hasPosition;
}

bool PlausibilityHelpers::isMultiwayPot(const CurrentHandContext& ctx)
{
    return ctx.commonContext.playersContext.actingPlayersList->size() > 2;
}

bool PlausibilityHelpers::isDonkBet(const CurrentHandContext& ctx)
{
    return !ctx.personalContext.hasPosition && !ctx.personalContext.actions.flopIsAggressor &&
           ctx.commonContext.bettingContext.flopBetsOrRaisesNumber > 0;
}

// Hand strength evaluation helpers

bool PlausibilityHelpers::handDoesNotUseHoleCards(const PostFlopAnalysisFlags& hand)
{
    return !hand.usesFirst && !hand.usesSecond;
}

bool PlausibilityHelpers::hasStrongHand(const PostFlopAnalysisFlags& hand)
{
    return (hand.isTwoPair && !hand.isFullHousePossible) || hand.isStraight || hand.isFlush || hand.isFullHouse ||
           hand.isTrips || hand.isQuads || hand.isStFlush;
}

bool PlausibilityHelpers::hasStrongDraw(const PostFlopAnalysisFlags& hand)
{
    return hand.flushOuts >= 8 || hand.straightOuts >= 8;
}

bool PlausibilityHelpers::hasWeakHand(const PostFlopAnalysisFlags& hand)
{
    return hand.isNoPair || (hand.isOnePair && !hand.isTopPair && !hand.isOverPair && !hand.isFullHousePossible);
}

bool PlausibilityHelpers::hasTopPairOrBetter(const PostFlopAnalysisFlags& hand)
{
    return hand.isTopPair || hand.isOverPair || hand.isTwoPair || hand.isTrips || hand.isStraight || hand.isFlush ||
           hand.isFullHouse || hand.isQuads || hand.isStFlush;
}

bool PlausibilityHelpers::hasTwoPairOrBetter(const PostFlopAnalysisFlags& hand)
{
    return (hand.isTwoPair && !hand.isFullHousePossible) || hand.isStraight || hand.isFlush || hand.isFullHouse ||
           hand.isTrips || hand.isQuads || hand.isStFlush;
}

// Board texture helpers

bool PlausibilityHelpers::isDangerousBoard(const PostFlopAnalysisFlags& hand)
{
    return hand.isFlushDrawPossible || hand.isStraightDrawPossible;
}

bool PlausibilityHelpers::isPairedBoard(const PostFlopAnalysisFlags& hand)
{
    // For now, we'll use isFullHousePossible as an indicator that the board might be paired
    // since full house is only possible with a paired board
    return hand.isFullHousePossible;
}

// Statistics helpers

bool PlausibilityHelpers::hasInsufficientStatistics(const CurrentHandContext& ctx)
{
    const auto& flop = ctx.personalContext.statistics.flopStatistics;
    return flop.hands < MIN_HANDS_STATISTICS_ACCURATE;
}

bool PlausibilityHelpers::isPassivePlayer(const CurrentHandContext& ctx)
{
    const auto& flop = ctx.personalContext.statistics.flopStatistics;
    return flop.getAgressionFactor() < 2 && flop.getAgressionFrequency() < 30 &&
           flop.hands > MIN_HANDS_STATISTICS_ACCURATE;
}

// Betting round specific helpers

bool PlausibilityHelpers::isAggressiveOnRound(const CurrentHandContext& ctx, const std::string& round)
{
    if (round == "flop")
    {
        const auto& flop = ctx.personalContext.statistics.flopStatistics;
        return flop.getAgressionFactor() > 3 && flop.getAgressionFrequency() > 50 &&
               flop.hands > MIN_HANDS_STATISTICS_ACCURATE;
    }
    else if (round == "turn")
    {
        const auto& turn = ctx.personalContext.statistics.turnStatistics;
        return turn.getAgressionFactor() > 3 && turn.getAgressionFrequency() > 50 &&
               turn.hands > MIN_HANDS_STATISTICS_ACCURATE;
    }
    else if (round == "river")
    {
        const auto& river = ctx.personalContext.statistics.riverStatistics;
        return river.getAgressionFactor() > 3 && river.getAgressionFrequency() > 50 &&
               river.hands > MIN_HANDS_STATISTICS_ACCURATE;
    }
    return false;
}

} // namespace pkt::core::player