#include "TurnPlausibilityChecker.h"
#include "PlausibilityHelpers.h"
#include "core/player/range/RangeEstimator.h"
#include "core/player/typedefs.h"

namespace pkt::core::player
{

bool TurnPlausibilityChecker::isUnplausibleHandGivenTurnCheck(const PostFlopAnalysisFlags& testedHand,
                                                              const CurrentHandContext& ctx)
{
    // In position player with non-passive statistics should be more aggressive
    if (PlausibilityHelpers::isInPosition(ctx) && !PlausibilityHelpers::isPassivePlayer(ctx))
    {
        // Over pair should not check in position
        if (testedHand.isPocketPair && testedHand.isOverPair)
        {
            return true;
        }

        // Wouldn't slow play medium hands on dangerous board with no flop action
        if (((testedHand.usesFirst || testedHand.usesSecond) &&
                 ctx.commonContext.bettingContext.flopBetsOrRaisesNumber == 0 && testedHand.isTopPair ||
             PlausibilityHelpers::hasTwoPairOrBetter(testedHand)) &&
            PlausibilityHelpers::isDangerousBoard(testedHand))
        {
            return true;
        }

        // Wouldn't be passive with decent hand in position in multiway pot
        if (((testedHand.usesFirst || testedHand.usesSecond) && PlausibilityHelpers::hasTwoPairOrBetter(testedHand)) &&
            PlausibilityHelpers::isMultiwayPot(ctx))
        {
            return true;
        }
    }

    return false;
}

bool TurnPlausibilityChecker::isUnplausibleHandGivenTurnBet(const PostFlopAnalysisFlags& testedHand,
                                                            const CurrentHandContext& ctx)
{
    // Maniac players are unpredictable
    if (PlausibilityHelpers::isAggressiveOnRound(ctx, "turn") || PlausibilityHelpers::isManiacPlayer(ctx))
    {
        return false;
    }

    // Hand not using hole cards is implausible for betting
    if (PlausibilityHelpers::handDoesNotUseHoleCards(testedHand))
    {
        return true;
    }

    // Donk bet analysis - player should have strong hand for turn donk bet
    if (PlausibilityHelpers::isDonkBet(ctx))
    {
        // Weak draws in multiway pot are implausible for donk betting
        if ((testedHand.isOverCards || PlausibilityHelpers::hasStrongDraw(testedHand)) &&
            PlausibilityHelpers::isMultiwayPot(ctx))
        {
            return true;
        }

        // Should have at least strong hand for donk bet
        if (!PlausibilityHelpers::hasStrongHand(testedHand))
        {
            if (PlausibilityHelpers::hasWeakHand(testedHand))
            {
                return true;
            }
        }
    }

    return false;
}

bool TurnPlausibilityChecker::isUnplausibleHandGivenTurnCall(const PostFlopAnalysisFlags& testedHand,
                                                             const CurrentHandContext& ctx)
{

    auto& turn = ctx.personalContext.statistics.turnStatistics;

    if (ctx.commonContext.bettingContext.potOdd < 20)
    {
        return false;
    }

    if (PlausibilityHelpers::isManiacPlayer(ctx))
    {
        return false; // he is (temporarily ?) very loose, so don't make any guess
    }

    if (PlausibilityHelpers::handDoesNotUseHoleCards(testedHand))
    {
        return true;
    }

    // the player called a bet on flop and turn, and he is not loose
    if (ctx.commonContext.bettingContext.turnBetsOrRaisesNumber > 0 &&
        ctx.commonContext.bettingContext.flopBetsOrRaisesNumber > 0 &&
        ctx.personalContext.actions.currentHandActions.getActions(GameState::Turn).back().type == ActionType::Call &&
        !ctx.personalContext.actions.flopIsAggressor &&
        !(ctx.personalContext.statistics.getWentToShowDown() > 30 &&
          ctx.personalContext.statistics.riverStatistics.hands > MIN_HANDS_STATISTICS_ACCURATE))
    {

        if (!((testedHand.isTwoPair && !testedHand.isFullHousePossible) || testedHand.isStraight ||
              testedHand.isFlush || testedHand.isFullHouse || testedHand.isTrips || testedHand.isQuads ||
              testedHand.isStFlush || testedHand.flushOuts >= 8 || testedHand.straightOuts >= 8))
        {

            if (testedHand.isNoPair || (testedHand.isOnePair && testedHand.isFullHousePossible))
            {
                return true;
            }

            if (testedHand.isOnePair && !testedHand.isTopPair && !testedHand.isOverPair &&
                !testedHand.isFullHousePossible)
            {
                return true;
            }

            if (ctx.commonContext.bettingContext.turnBetsOrRaisesNumber > 2 && testedHand.isOnePair)
            {
                return true;
            }
        }
    }
    // the player called a raise on turn, and is not loose : he has at least a top pair or a good draw
    if (ctx.commonContext.bettingContext.turnBetsOrRaisesNumber > 1 &&
        ctx.personalContext.actions.currentHandActions.getActions(GameState::Turn).back().type == ActionType::Call &&
        !(ctx.personalContext.statistics.getWentToShowDown() > 35 &&
          ctx.personalContext.statistics.riverStatistics.hands > MIN_HANDS_STATISTICS_ACCURATE))
    {

        if (!((testedHand.isTwoPair && !testedHand.isFullHousePossible) || testedHand.isStraight ||
              testedHand.isFlush || testedHand.isFullHouse || testedHand.isTrips || testedHand.isQuads ||
              testedHand.isStFlush || testedHand.isOverCards || testedHand.isFlushDrawPossible ||
              testedHand.flushOuts >= 8 || testedHand.straightOuts >= 8))
        {

            if (testedHand.isNoPair || (testedHand.isOnePair && testedHand.isFullHousePossible))
            {
                return true;
            }

            if (testedHand.isOnePair && !testedHand.isTopPair && !testedHand.isFullHousePossible &&
                !testedHand.isOverPair)
            {
                return true;
            }
        }
    }

    return false;
}

bool TurnPlausibilityChecker::isUnplausibleHandGivenTurnRaise(const PostFlopAnalysisFlags& testedHand,
                                                              const CurrentHandContext& ctx)
{

    auto& turn = ctx.personalContext.statistics.turnStatistics;

    if (turn.getAgressionFactor() > 3 && turn.getAgressionFrequency() > 50 &&
        turn.hands > MIN_HANDS_STATISTICS_ACCURATE)
    {
        return false; // he is very agressive, so don't make any guess
    }

    if (PlausibilityHelpers::isManiacPlayer(ctx))
    {
        return false; // he is (temporarily ?) very agressive, so don't make any guess
    }

    if (PlausibilityHelpers::handDoesNotUseHoleCards(testedHand))
    {
        return true;
    }

    // if nobody has bet the flop, he should at least have a top pair
    if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber == 0)
    {

        if (testedHand.isTopPair || testedHand.isOverPair ||
            (testedHand.isTwoPair && !testedHand.isFullHousePossible) || testedHand.isStraight || testedHand.isFlush ||
            testedHand.isFullHouse || testedHand.isTrips || testedHand.isQuads || testedHand.isStFlush)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    // if he was not the agressor on flop, and an other player has bet the flop, then he should have at least a top pair
    if (!ctx.personalContext.actions.flopIsAggressor && ctx.commonContext.bettingContext.flopBetsOrRaisesNumber > 0)
    {

        if (testedHand.isTopPair || testedHand.isOverPair ||
            (testedHand.isTwoPair && !testedHand.isFullHousePossible) || testedHand.isStraight || testedHand.isFlush ||
            testedHand.isFullHouse || testedHand.isTrips || testedHand.isQuads || testedHand.isStFlush)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    // the player has raised twice the turn, and is not a maniac player : he should have at least two pairs
    if (ctx.personalContext.actions.currentHandActions.getActionsNumber(GameState::Turn, ActionType::Raise) == 2 &&
        !((testedHand.isTwoPair && !testedHand.isFullHousePossible) || testedHand.isStraight || testedHand.isFlush ||
          testedHand.isFullHouse || testedHand.isTrips || testedHand.isQuads || testedHand.isStFlush))
    {
        return true;
    }

    // the player has raised 3 times the turn, and is not a maniac player : he should have better than a set
    if (ctx.personalContext.actions.currentHandActions.getActionsNumber(GameState::Turn, ActionType::Raise) > 2 &&
        !(testedHand.isStraight || testedHand.isFlush || testedHand.isFullHouse || testedHand.isQuads ||
          testedHand.isStFlush))
    {
        return true;
    }

    return false;
}

bool TurnPlausibilityChecker::isUnplausibleHandGivenTurnAllin(const PostFlopAnalysisFlags& testedHand,
                                                              const CurrentHandContext& ctx)
{

    auto& turn = ctx.personalContext.statistics.turnStatistics;

    if (ctx.commonContext.bettingContext.potOdd < 20)
    {
        return false;
    }

    if (turn.getAgressionFactor() > 3 && turn.getAgressionFrequency() > 50 &&
        turn.hands > MIN_HANDS_STATISTICS_ACCURATE)
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

    // the player has raised twice the turn, and is not a maniac player : he should have at least two pairs
    if (ctx.personalContext.actions.currentHandActions.getActionsNumber(GameState::Turn, ActionType::Raise) == 2 &&
        !((testedHand.isTwoPair && !testedHand.isFullHousePossible) || testedHand.isStraight || testedHand.isFlush ||
          testedHand.isFullHouse || testedHand.isTrips || testedHand.isQuads || testedHand.isStFlush))
    {
        return true;
    }

    // the player has raised 3 times the turn, and is not a maniac player : he should have better than a set
    if (ctx.personalContext.actions.currentHandActions.getActionsNumber(GameState::Turn, ActionType::Raise) > 2 &&
        !(testedHand.isStraight || testedHand.isFlush || testedHand.isFullHouse || testedHand.isQuads ||
          testedHand.isStFlush))
    {
        return true;
    }

    return false;
}

} // namespace pkt::core::player
