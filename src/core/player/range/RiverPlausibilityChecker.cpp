#include "RiverPlausibilityChecker.h"
#include "PlausibilityHelpers.h"
#include "core/player/range/RangeEstimator.h"
#include "core/player/typedefs.h"

namespace pkt::core::player
{

bool RiverPlausibilityChecker::isUnplausibleHandGivenRiverCheck(const PostFlopAnalysisFlags& testedHand,
                                                                const CurrentHandContext& ctx)
{
    // TODO implement river check analysis
    return false;
}

bool RiverPlausibilityChecker::isUnplausibleHandGivenRiverBet(const PostFlopAnalysisFlags& testedHand,
                                                              const CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.personalContext.hasPosition;
    auto& river = ctx.personalContext.statistics.riverStatistics;

    if (river.getAgressionFactor() > 3 && river.getAgressionFrequency() > 50 &&
        river.hands > MIN_HANDS_STATISTICS_ACCURATE)
    {
        return false; // he is usually very agressive, so don't make any guess
    }

    if (PlausibilityHelpers::isManiacPlayer(ctx))
    {
        return false; // he is (temporarily ?) very agressive, so don't make any guess
    }

    if (PlausibilityHelpers::handDoesNotUseHoleCards(testedHand))
    {
        return true;
    }

    // the player has bet the river, was not the agressor on turn and river, and is not a maniac player : he should
    // have at least 2 pairs
    if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber > 1 && !ctx.personalContext.actions.flopIsAggressor &&
        ctx.commonContext.bettingContext.turnBetsOrRaisesNumber > 1 && !ctx.personalContext.actions.turnIsAggressor)
    {
        if (PlausibilityHelpers::hasPremiumHand(testedHand))
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    // the player has bet the river, is out of position on a multi-players pot, in a hand with some action, and is
    // not a maniac player : he should have at least 2 pairs
    if (PlausibilityHelpers::isOutOfPositionInMultiwayWithPriorAction(ctx))
    {
        if (PlausibilityHelpers::hasPremiumHand(testedHand))
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    return false;
}

bool RiverPlausibilityChecker::isUnplausibleHandGivenRiverCall(const PostFlopAnalysisFlags& testedHand,
                                                               const CurrentHandContext& ctx)
{

    auto& river = ctx.personalContext.statistics.riverStatistics;

    if (ctx.commonContext.bettingContext.potOdd < 20)
    {
        return false;
    }

    if (PlausibilityHelpers::isAggressiveOnRiver(ctx))
    {
        return false; // he is usually very agressive, so don't make any guess
    }

    if (PlausibilityHelpers::isManiacPlayer(ctx))
    {
        return false; // he is (temporarily ?) very agressive, so don't make any guess
    }

    if (PlausibilityHelpers::handDoesNotUseHoleCards(testedHand))
    {
        return true;
    }

    // the player has called the river on a multi-players pot, and is not a loose player : he should have at least a
    // top pair
    if (ctx.commonContext.playersContext.actingPlayersList->size() > 2)
    {
        if (!PlausibilityHelpers::hasPremiumHand(testedHand))
        {
            if (testedHand.isNoPair || PlausibilityHelpers::hasWeakPairOnPairedBoard(testedHand))
            {
                return true;
            }

            if (testedHand.isOnePair && !testedHand.isTopPair && !testedHand.isOverPair)
            {
                return true;
            }
        }
    }

    return false;
}

bool RiverPlausibilityChecker::isUnplausibleHandGivenRiverRaise(const PostFlopAnalysisFlags& testedHand,
                                                                const CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.personalContext.hasPosition;
    auto& river = ctx.personalContext.statistics.riverStatistics;

    if (river.getAgressionFactor() > 3 && river.getAgressionFrequency() > 50 &&
        river.hands > MIN_HANDS_STATISTICS_ACCURATE)
    {
        return false; // he is usually very agressive, so don't make any guess
    }

    if (PlausibilityHelpers::isManiacPlayer(ctx))
    {
        return false; // he is (temporarily ?) very agressive, so don't make any guess
    }

    // the player has raised the river, and is not a maniac player : he should have at least 2 pairs
    if (PlausibilityHelpers::handDoesNotUseHoleCards(testedHand))
    {
        return true;
    }

    if (PlausibilityHelpers::hasPremiumHand(testedHand))
    {
        return false;
    }
    else
    {
        return true;
    }

    // the player has raised the river, is out of position on a multi-players pot, in a hand with some action, and
    // is not a maniac player : he should have at least a set
    if (PlausibilityHelpers::isOutOfPositionInMultiwayWithPriorAction(ctx))
    {
        if (PlausibilityHelpers::hasStrongHandExcludingTwoPair(testedHand))
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    // the player has raised twice the river, and is not a maniac player : he should have at least trips
    if (ctx.personalContext.actions.currentHandActions.getActionsNumber(GameState::River, ActionType::Raise) == 2 &&
        !PlausibilityHelpers::hasStrongHandExcludingTwoPair(testedHand))
    {
        return true;
    }

    // the player has raised 3 times the river, and is not a maniac player : he should have better than a set
    if (ctx.personalContext.actions.currentHandActions.getActionsNumber(GameState::River, ActionType::Raise) > 2 &&
        !PlausibilityHelpers::hasVeryStrongHand(testedHand))
    {
        return true;
    }

    return false;
}

bool RiverPlausibilityChecker::isUnplausibleHandGivenRiverAllin(const PostFlopAnalysisFlags& testedHand,
                                                                const CurrentHandContext& ctx)
{

    auto& river = ctx.personalContext.statistics.riverStatistics;

    if (ctx.commonContext.bettingContext.potOdd < 20)
    {
        return false;
    }

    if (PlausibilityHelpers::isAggressiveOnRiver(ctx))
    {
        return false; // he is usually very agressive, so don't make any guess
    }

    if (PlausibilityHelpers::isManiacPlayer(ctx))
    {
        return false; // he is (temporarily ?) very agressive, so don't make any guess
    }

    if (PlausibilityHelpers::handDoesNotUseHoleCards(testedHand))
    {
        return true;
    }

    // the player has raised twice or more the river, and is not a maniac player : he should have at least a
    // straight
    if (ctx.personalContext.actions.currentHandActions.getActionsNumber(GameState::River, ActionType::Raise) > 1 &&
        !PlausibilityHelpers::hasVeryStrongHand(testedHand))
    {
        return true;
    }

    return false;
}

} // namespace pkt::core::player
