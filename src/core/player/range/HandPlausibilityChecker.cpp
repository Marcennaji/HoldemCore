
#include "HandPlausibilityChecker.h"
#include "core/player/range/RangeEstimator.h"
#include "core/player/typedefs.h"

namespace pkt::core::player
{

bool HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(const PostFlopAnalysisFlags& testedHand,
                                                              CurrentHandContext& ctx)
{

    const int nbPlayers = ctx.commonContext.nbPlayers;
    const bool bHavePosition = ctx.perPlayerContext.myHavePosition;
    auto& flop = ctx.perPlayerContext.myStatistics.getFlopStatistics();

    // the player is in position, he didn't bet on flop, he is not usually passive, and everybody checked on flop :

    if (bHavePosition &&
        !(flop.getAgressionFactor() < 2 && flop.getAgressionFrequency() < 30 &&
          flop.m_hands > MIN_HANDS_STATISTICS_ACCURATE) &&
        (testedHand.usesFirst || testedHand.usesSecond))
    {

        // woudn't slow play a medium hand on a dangerous board
        if (!testedHand.isFullHousePossible &&
            ((testedHand.isMiddlePair && !testedHand.isFullHousePossible && ctx.commonContext.nbRunningPlayers < 4) ||
             testedHand.isTopPair || testedHand.isOverPair ||
             (testedHand.isTwoPair && !testedHand.isFullHousePossible)) &&
            testedHand.isFlushDrawPossible && testedHand.isStraightDrawPossible)
        {
            return true;
        }

        // on a non-paired board, he would'nt slow play a straigth, a set or 2 pairs, if a flush draw is possible
        if (!testedHand.isFullHousePossible && (testedHand.isTrips || testedHand.isStraight || testedHand.isTwoPair) &&
            testedHand.isFlushDrawPossible)
        {
            return true;
        }

        // wouldn't be passive with a decent hand, on position, if more than 1 opponent
        if (!testedHand.isFullHousePossible &&
            (testedHand.isTopPair || testedHand.isOverPair || testedHand.isTwoPair || testedHand.isTrips) &&
            ctx.commonContext.nbRunningPlayers > 2)
        {
            return true;
        }

        // on a paired board, he wouldn't check if he has a pocket overpair
        if (testedHand.isFullHousePossible && testedHand.isOverPair)
        {
            return true;
        }
    }
    return false;
}

bool HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(const PostFlopAnalysisFlags& testedHand,
                                                            CurrentHandContext& ctx)
{

    const int nbPlayers = ctx.commonContext.nbPlayers;
    const bool bHavePosition = ctx.perPlayerContext.myHavePosition;
    auto& flop = ctx.perPlayerContext.myStatistics.getFlopStatistics();

    if (flop.getAgressionFactor() > 3 && flop.getAgressionFrequency() > 50 &&
        flop.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
    {
        return false; // he is very agressive, so don't make any guess
    }

    if (ctx.perPlayerContext.myIsInVeryLooseMode)
    {
        return false; // he is (temporarily ?) very agressive, so don't make any guess
    }

    if (!testedHand.usesFirst && !testedHand.usesSecond)
    {
        return true;
    }

    // the player made a donk bet on the flop, and is not a maniac player : he should have at least a middle or top pair
    // or a draw
    if (!bHavePosition && !ctx.perPlayerContext.myPreflopIsAggressor)
    {

        if (testedHand.isOverCards || testedHand.straightOuts >= 8 || testedHand.flushOuts >= 8)
        {
            return (ctx.commonContext.nbRunningPlayers > 2 ? true : false);
        }

        if (!((testedHand.isTwoPair && !testedHand.isFullHousePossible) || testedHand.isStraight ||
              testedHand.isFlush || testedHand.isFullHouse || testedHand.isTrips || testedHand.isQuads ||
              testedHand.isStFlush))
        {

            if (testedHand.isNoPair)
            {
                return true;
            }

            if (testedHand.isOnePair)
            {

                if (testedHand.isFullHousePossible)
                {
                    return true;
                }

                if (!testedHand.isMiddlePair && !testedHand.isTopPair && !testedHand.isOverPair)
                {
                    return true;
                }
            }
        }
    }

    // on a 3 or more players pot : if the player bets in position, he should have at least a middle pair
    if (bHavePosition && ctx.commonContext.nbRunningPlayers > 2)
    {

        if (testedHand.isOverCards || testedHand.straightOuts >= 8 || testedHand.flushOuts >= 8)
        {
            return true;
        }

        if (!((testedHand.isTwoPair && !testedHand.isFullHousePossible) || testedHand.isStraight ||
              testedHand.isFlush || testedHand.isFullHouse || testedHand.isTrips || testedHand.isQuads ||
              testedHand.isStFlush))
        {

            if (testedHand.isNoPair)
            {
                return true;
            }

            if (testedHand.isOnePair)
            {

                if (testedHand.isFullHousePossible)
                {
                    return true;
                }

                if (!testedHand.isMiddlePair && !testedHand.isTopPair && !testedHand.isOverPair)
                {
                    return true;
                }
            }
        }
    }

    // on a 3 or more players pot : if the player is first to act, and bets, he should have at least a top pair
    if (ctx.perPlayerContext.nbChecks == 0 && ctx.commonContext.nbRunningPlayers > 2)
    {

        if (!((testedHand.isTwoPair && !testedHand.isFullHousePossible) || testedHand.isStraight ||
              testedHand.isFlush || testedHand.isFullHouse || testedHand.isTrips || testedHand.isQuads ||
              testedHand.isStFlush))
        {

            if (testedHand.isNoPair)
            {
                return true;
            }

            if (testedHand.isOnePair)
            {

                if (testedHand.isFullHousePossible)
                {
                    return true;
                }

                if (!testedHand.isTopPair && !testedHand.isOverPair)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool HandPlausibilityChecker::isUnplausibleHandGivenFlopCall(const PostFlopAnalysisFlags& testedHand,
                                                             CurrentHandContext& ctx)
{

    const int nbPlayers = ctx.commonContext.nbPlayers;
    const bool bHavePosition = ctx.perPlayerContext.myHavePosition;
    auto& flop = ctx.perPlayerContext.myStatistics.getFlopStatistics();

    if (ctx.commonContext.potOdd < 20)
    {
        return false;
    }

    if (ctx.perPlayerContext.myIsInVeryLooseMode)
    {
        return false; // he is (temporarily ?) very agressive, so don't make any guess
    }

    if (!testedHand.usesFirst && !testedHand.usesSecond)
    {
        return true;
    }

    if (ctx.commonContext.flopBetsOrRaisesNumber > 0 &&
        ctx.perPlayerContext.myCurrentHandActions.getFlopActions().back() == ActionType::Call &&
        !(ctx.perPlayerContext.myStatistics.getWentToShowDown() > 35 &&
          ctx.perPlayerContext.myStatistics.getRiverStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE))
    {

        if (!((testedHand.isTwoPair && !testedHand.isFullHousePossible) || testedHand.isStraight ||
              testedHand.isFlush || testedHand.isFullHouse || testedHand.isTrips || testedHand.isQuads ||
              testedHand.isStFlush || testedHand.isOverCards || testedHand.flushOuts >= 8 ||
              testedHand.straightOuts >= 8))
        {

            if (testedHand.isNoPair)
            {
                return true;
            }

            if (ctx.commonContext.flopBetsOrRaisesNumber > 1 && testedHand.isOnePair && !testedHand.isTopPair &&
                !testedHand.isOverPair)
            {
                return true;
            }

            if (ctx.commonContext.flopBetsOrRaisesNumber > 2 && (testedHand.isOnePair || testedHand.isOverCards))
            {
                return true;
            }

            if (ctx.commonContext.nbRunningPlayers > 2 && testedHand.isOnePair && !testedHand.isTopPair &&
                !testedHand.isOverPair)
            {
                return true;
            }
        }
    }
    return false;
}

bool HandPlausibilityChecker::isUnplausibleHandGivenFlopRaise(const PostFlopAnalysisFlags& testedHand,
                                                              CurrentHandContext& ctx)
{

    const int nbPlayers = ctx.commonContext.nbPlayers;
    const bool bHavePosition = ctx.perPlayerContext.myHavePosition;
    auto& flop = ctx.perPlayerContext.myStatistics.getFlopStatistics();

    if (flop.getAgressionFactor() > 3 && flop.getAgressionFrequency() > 50 &&
        flop.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
    {
        return false; // he is usually very agressive, so don't make any guess
    }

    if (ctx.perPlayerContext.myIsInVeryLooseMode)
    {
        return false; // he is (temporarily ?) very agressive, so don't make any guess
    }

    if (!testedHand.usesFirst && !testedHand.usesSecond)
    {
        return true;
    }

    // the player has check-raised the flop, and is not a maniac player : he should have at least a top pair or a draw
    if (ctx.perPlayerContext.nbChecks == 1)
    {

        if ((testedHand.isOverCards || testedHand.flushOuts >= 8 || testedHand.straightOuts >= 8) &&
            ctx.commonContext.nbRunningPlayers > 2)
        {
            return true;
        }

        if (!((testedHand.isTwoPair && !testedHand.isFullHousePossible) || testedHand.isStraight ||
              testedHand.isFlush || testedHand.isFullHouse || testedHand.isTrips || testedHand.isQuads ||
              testedHand.isStFlush))
        {

            if (testedHand.isNoPair)
            {
                return true;
            }

            if (testedHand.isOnePair && !testedHand.isFullHousePossible && !testedHand.isTopPair &&
                !testedHand.isOverPair)
            {
                return true;
            }
        }
    }

    // the player has raised or reraised the flop, and is not a maniac player : he should have at least a top pair
    if (ctx.perPlayerContext.nbRaises > 0)
    {

        if (!((testedHand.isTwoPair && !testedHand.isFullHousePossible) || testedHand.isStraight ||
              testedHand.isFlush || testedHand.isFullHouse || testedHand.isTrips || testedHand.isQuads ||
              testedHand.isStFlush))
        {

            if (testedHand.isNoPair || (testedHand.isOnePair && testedHand.isFullHousePossible))
            {
                return true;
            }

            if (testedHand.isOnePair && !testedHand.isFullHousePossible &&
                !testedHand.isTopPair & !testedHand.isOverPair)
            {
                return true;
            }

            if (ctx.commonContext.flopBetsOrRaisesNumber > 3 && (testedHand.isOnePair))
            {
                return true;
            }

            if (ctx.commonContext.flopBetsOrRaisesNumber > 4 && (testedHand.isTwoPair))
            {
                return true;
            }
        }
    }

    return false;
}

bool HandPlausibilityChecker::isUnplausibleHandGivenFlopAllin(const PostFlopAnalysisFlags& testedHand,
                                                              CurrentHandContext& ctx)
{

    const int nbPlayers = ctx.commonContext.nbPlayers;
    const bool bHavePosition = ctx.perPlayerContext.myHavePosition;
    auto& flop = ctx.perPlayerContext.myStatistics.getFlopStatistics();

    if (ctx.commonContext.potOdd < 20)
    {
        return false;
    }

    if (flop.getAgressionFactor() > 3 && flop.getAgressionFrequency() > 50 &&
        flop.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
    {
        return false; // he is usually very agressive, so don't make any guess
    }

    if (ctx.perPlayerContext.myIsInVeryLooseMode)
    {
        return false; // he is (temporarily ?) very agressive, so don't make any guess
    }

    if (!testedHand.usesFirst && !testedHand.usesSecond)
    {
        return true;
    }

    if (!((testedHand.isTwoPair && !testedHand.isFullHousePossible) || testedHand.isStraight || testedHand.isFlush ||
          testedHand.isFullHouse || testedHand.isTrips || testedHand.isQuads || testedHand.isStFlush))
    {

        if (testedHand.isNoPair || (testedHand.isOnePair && testedHand.isFullHousePossible))
        {
            return true;
        }

        if (testedHand.isOnePair && !testedHand.isFullHousePossible && !testedHand.isTopPair & !testedHand.isOverPair)
        {
            return true;
        }

        if (ctx.commonContext.flopBetsOrRaisesNumber > 3 && (testedHand.isOnePair))
        {
            return true;
        }

        if (ctx.commonContext.flopBetsOrRaisesNumber > 4 && (testedHand.isTwoPair))
        {
            return true;
        }
    }
    return false;
}

bool HandPlausibilityChecker::isUnplausibleHandGivenTurnCheck(const PostFlopAnalysisFlags& testedHand,
                                                              CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.perPlayerContext.myHavePosition;
    auto& turn = ctx.perPlayerContext.myStatistics.getTurnStatistics();

    // the player is in position, he isn't usually passive, and everybody checked
    if (bHavePosition && !(turn.getAgressionFactor() < 2 && turn.getAgressionFrequency() < 30 &&
                           turn.m_hands > MIN_HANDS_STATISTICS_ACCURATE))
    {

        if (testedHand.isPocketPair && testedHand.isOverPair)
        {
            return true;
        }

        // woudn't slow play a medium hand on a dangerous board, if there was no action on flop
        if (((testedHand.usesFirst || testedHand.usesSecond) && ctx.commonContext.flopBetsOrRaisesNumber == 0 &&
                 testedHand.isTopPair ||
             (testedHand.isTwoPair && !testedHand.isFullHousePossible) || testedHand.isTrips) &&
            testedHand.isFlushDrawPossible)
        {
            return true;
        }

        // wouldn't be passive with a decent hand, on position, if more than 1 opponent
        if (((testedHand.usesFirst || testedHand.usesSecond) &&
             ((testedHand.isTwoPair && !testedHand.isFullHousePossible) || testedHand.isTrips)) &&
            ctx.commonContext.nbRunningPlayers > 2)
        {
            return true;
        }
    }

    return false;
}

bool HandPlausibilityChecker::isUnplausibleHandGivenTurnBet(const PostFlopAnalysisFlags& testedHand,
                                                            CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.perPlayerContext.myHavePosition;
    auto& turn = ctx.perPlayerContext.myStatistics.getTurnStatistics();

    if (turn.getAgressionFactor() > 3 && turn.getAgressionFrequency() > 50 &&
        turn.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
    {
        return false; // he is usually very agressive, so don't make any guess
    }

    if (ctx.perPlayerContext.myIsInVeryLooseMode)
    {
        return false; // he is (temporarily ?) very agressive, so don't make any guess
    }

    if (!testedHand.usesFirst && !testedHand.usesSecond)
    {
        return true;
    }

    // the player made a donk bet on turn, and is not a maniac player : he should have at least a top pair
    if (!bHavePosition && !ctx.perPlayerContext.myFlopIsAggressor && ctx.commonContext.flopBetsOrRaisesNumber > 0)
    {

        if ((testedHand.isOverCards || testedHand.flushOuts >= 8 || testedHand.straightOuts >= 8) &&
            ctx.commonContext.nbRunningPlayers > 2)
        {
            return true;
        }

        if (!((testedHand.isTwoPair && !testedHand.isFullHousePossible) || testedHand.isStraight ||
              testedHand.isFlush || testedHand.isFullHouse || testedHand.isTrips || testedHand.isQuads ||
              testedHand.isStFlush))
        {

            if (testedHand.isNoPair)
            {
                return true;
            }

            if (testedHand.isOnePair && !testedHand.isTopPair && !testedHand.isOverPair &&
                !testedHand.isFullHousePossible)
            {
                return true;
            }
        }
    }

    return false;
}

bool HandPlausibilityChecker::isUnplausibleHandGivenTurnCall(const PostFlopAnalysisFlags& testedHand,
                                                             CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.perPlayerContext.myHavePosition;
    auto& turn = ctx.perPlayerContext.myStatistics.getTurnStatistics();

    if (ctx.commonContext.potOdd < 20)
    {
        return false;
    }

    if (ctx.perPlayerContext.myIsInVeryLooseMode)
    {
        return false; // he is (temporarily ?) very loose, so don't make any guess
    }

    if (!testedHand.usesFirst && !testedHand.usesSecond)
    {
        return true;
    }

    // the player called a bet on flop and turn, and he is not loose
    if (ctx.commonContext.turnBetsOrRaisesNumber > 0 && ctx.commonContext.flopBetsOrRaisesNumber > 0 &&
        ctx.perPlayerContext.myCurrentHandActions.getTurnActions().back() == ActionType::Call &&
        !ctx.perPlayerContext.myFlopIsAggressor &&
        !(ctx.perPlayerContext.myStatistics.getWentToShowDown() > 30 &&
          ctx.perPlayerContext.myStatistics.getRiverStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE))
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

            if (ctx.commonContext.turnBetsOrRaisesNumber > 2 && testedHand.isOnePair)
            {
                return true;
            }
        }
    }
    // the player called a raise on turn, and is not loose : he has at least a top pair or a good draw
    if (ctx.commonContext.turnBetsOrRaisesNumber > 1 &&
        ctx.perPlayerContext.myCurrentHandActions.getTurnActions().back() == ActionType::Call &&
        !(ctx.perPlayerContext.myStatistics.getWentToShowDown() > 35 &&
          ctx.perPlayerContext.myStatistics.getRiverStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE))
    {

        if (!(((testedHand.isTwoPair && !testedHand.isFullHousePossible) && !testedHand.isFullHousePossible) ||
              testedHand.isStraight || testedHand.isFlush || testedHand.isFullHouse || testedHand.isTrips ||
              testedHand.isQuads || testedHand.isStFlush || testedHand.isOverCards || testedHand.isFlushDrawPossible ||
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

bool HandPlausibilityChecker::isUnplausibleHandGivenTurnRaise(const PostFlopAnalysisFlags& testedHand,
                                                              CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.perPlayerContext.myHavePosition;
    auto& turn = ctx.perPlayerContext.myStatistics.getTurnStatistics();

    if (turn.getAgressionFactor() > 3 && turn.getAgressionFrequency() > 50 &&
        turn.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
    {
        return false; // he is very agressive, so don't make any guess
    }

    if (ctx.perPlayerContext.myIsInVeryLooseMode)
    {
        return false; // he is (temporarily ?) very agressive, so don't make any guess
    }

    if (!testedHand.usesFirst && !testedHand.usesSecond)
    {
        return true;
    }

    // if nobody has bet the flop, he should at least have a top pair
    if (ctx.commonContext.flopBetsOrRaisesNumber == 0)
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
    if (!ctx.perPlayerContext.myFlopIsAggressor && ctx.commonContext.flopBetsOrRaisesNumber > 0)
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
    if (ctx.perPlayerContext.nbRaises == 2 &&
        !((testedHand.isTwoPair && !testedHand.isFullHousePossible) || testedHand.isStraight || testedHand.isFlush ||
          testedHand.isFullHouse || testedHand.isTrips || testedHand.isQuads || testedHand.isStFlush))
    {
        return true;
    }

    // the player has raised 3 times the turn, and is not a maniac player : he should have better than a set
    if (ctx.perPlayerContext.nbRaises > 2 && !(testedHand.isStraight || testedHand.isFlush || testedHand.isFullHouse ||
                                               testedHand.isQuads || testedHand.isStFlush))
    {
        return true;
    }

    return false;
}

bool HandPlausibilityChecker::isUnplausibleHandGivenTurnAllin(const PostFlopAnalysisFlags& testedHand,
                                                              CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.perPlayerContext.myHavePosition;
    auto& turn = ctx.perPlayerContext.myStatistics.getTurnStatistics();

    if (ctx.commonContext.potOdd < 20)
    {
        return false;
    }

    if (turn.getAgressionFactor() > 3 && turn.getAgressionFrequency() > 50 &&
        turn.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
    {
        return false; // he is usually very agressive, so don't make any guess
    }

    if (ctx.perPlayerContext.myIsInVeryLooseMode)
    {
        return false; // he is (temporarily ?) very agressive, so don't make any guess
    }

    if (!testedHand.usesFirst && !testedHand.usesSecond)
    {
        return true;
    }

    // the player has raised twice the turn, and is not a maniac player : he should have at least two pairs
    if (ctx.perPlayerContext.nbRaises == 2 &&
        !((testedHand.isTwoPair && !testedHand.isFullHousePossible) || testedHand.isStraight || testedHand.isFlush ||
          testedHand.isFullHouse || testedHand.isTrips || testedHand.isQuads || testedHand.isStFlush))
    {
        return true;
    }

    // the player has raised 3 times the turn, and is not a maniac player : he should have better than a set
    if (ctx.perPlayerContext.nbRaises > 2 && !(testedHand.isStraight || testedHand.isFlush || testedHand.isFullHouse ||
                                               testedHand.isQuads || testedHand.isStFlush))
    {
        return true;
    }

    return false;
}

bool HandPlausibilityChecker::isUnplausibleHandGivenRiverCheck(const PostFlopAnalysisFlags& testedHand,
                                                               CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.perPlayerContext.myHavePosition;
    auto& river = ctx.perPlayerContext.myStatistics.getRiverStatistics();

    // todo

    return false;
}

bool HandPlausibilityChecker::isUnplausibleHandGivenRiverBet(const PostFlopAnalysisFlags& testedHand,
                                                             CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.perPlayerContext.myHavePosition;
    auto& river = ctx.perPlayerContext.myStatistics.getRiverStatistics();

    if (river.getAgressionFactor() > 3 && river.getAgressionFrequency() > 50 &&
        river.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
    {
        return false; // he is usually very agressive, so don't make any guess
    }

    if (ctx.perPlayerContext.myIsInVeryLooseMode)
    {
        return false; // he is (temporarily ?) very agressive, so don't make any guess
    }

    if (!testedHand.usesFirst && !testedHand.usesSecond)
    {
        return true;
    }

    // the player has bet the river, was not the agressor on turn and river, and is not a maniac player : he should
    // have at least 2 pairs
    if (ctx.commonContext.flopBetsOrRaisesNumber > 1 && !ctx.perPlayerContext.myFlopIsAggressor &&
        ctx.commonContext.turnBetsOrRaisesNumber > 1 && !ctx.perPlayerContext.myTurnIsAggressor)
    {

        if ((testedHand.isTwoPair && !testedHand.isFullHousePossible) || testedHand.isStraight || testedHand.isFlush ||
            testedHand.isFullHouse || testedHand.isTrips || testedHand.isQuads || testedHand.isStFlush)
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
    if (!bHavePosition && ctx.commonContext.nbRunningPlayers > 2 &&
        ((ctx.commonContext.flopBetsOrRaisesNumber > 1 && !ctx.perPlayerContext.myFlopIsAggressor) ||
         (ctx.commonContext.turnBetsOrRaisesNumber > 1 && !ctx.perPlayerContext.myTurnIsAggressor)))
    {

        if ((testedHand.isTwoPair && !testedHand.isFullHousePossible) || testedHand.isStraight || testedHand.isFlush ||
            testedHand.isFullHouse || testedHand.isTrips || testedHand.isQuads || testedHand.isStFlush)
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

bool HandPlausibilityChecker::isUnplausibleHandGivenRiverCall(const PostFlopAnalysisFlags& testedHand,
                                                              CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.perPlayerContext.myHavePosition;
    auto& river = ctx.perPlayerContext.myStatistics.getRiverStatistics();

    if (ctx.commonContext.potOdd < 20)
    {
        return false;
    }

    if (river.getAgressionFactor() > 3 && river.getAgressionFrequency() > 50 &&
        river.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
    {
        return false; // he is usually very agressive, so don't make any guess
    }

    if (ctx.perPlayerContext.myIsInVeryLooseMode)
    {
        return false; // he is (temporarily ?) very agressive, so don't make any guess
    }

    if (!testedHand.usesFirst && !testedHand.usesSecond)
    {
        return true;
    }

    // the player has called the river on a multi-players pot, and is not a loose player : he should have at least a
    // top pair
    if (ctx.commonContext.nbRunningPlayers > 2)
    {

        if (!((testedHand.isTwoPair && !testedHand.isFullHousePossible) || testedHand.isStraight ||
              testedHand.isFlush || testedHand.isFullHouse || testedHand.isTrips || testedHand.isQuads ||
              testedHand.isStFlush))
        {

            if (testedHand.isNoPair || (testedHand.isOnePair && testedHand.isFullHousePossible))
            {
                return true;
            }

            if (testedHand.isOnePair && !testedHand.isTopPair & !testedHand.isOverPair)
            {
                return true;
            }
        }
    }

    return false;
}

bool HandPlausibilityChecker::isUnplausibleHandGivenRiverRaise(const PostFlopAnalysisFlags& testedHand,
                                                               CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.perPlayerContext.myHavePosition;
    auto& river = ctx.perPlayerContext.myStatistics.getRiverStatistics();

    if (river.getAgressionFactor() > 3 && river.getAgressionFrequency() > 50 &&
        river.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
    {
        return false; // he is usually very agressive, so don't make any guess
    }

    if (ctx.perPlayerContext.myIsInVeryLooseMode)
    {
        return false; // he is (temporarily ?) very agressive, so don't make any guess
    }

    // the player has raised the river, and is not a maniac player : he should have at least 2 pairs
    if (!testedHand.usesFirst && !testedHand.usesSecond)
    {
        return true;
    }

    if (testedHand.isStraight || testedHand.isFlush || testedHand.isFullHouse || testedHand.isTrips ||
        testedHand.isQuads || testedHand.isStFlush || (testedHand.isTwoPair && !testedHand.isFullHousePossible))
    {
        return false;
    }
    else
    {
        return true;
    }

    // the player has raised the river, is out of position on a multi-players pot, in a hand with some action, and
    // is not a maniac player : he should have at least a set
    if (!bHavePosition && ctx.commonContext.nbRunningPlayers > 2 &&
        ((ctx.commonContext.flopBetsOrRaisesNumber > 1 && !ctx.perPlayerContext.myFlopIsAggressor) ||
         (ctx.commonContext.turnBetsOrRaisesNumber > 1 && !ctx.perPlayerContext.myTurnIsAggressor)))
    {

        if (testedHand.isStraight || testedHand.isFlush || testedHand.isFullHouse || testedHand.isTrips ||
            testedHand.isQuads || testedHand.isStFlush)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    // the player has raised twice the river, and is not a maniac player : he should have at least trips
    if (ctx.perPlayerContext.nbRaises == 2 && !(testedHand.isStraight || testedHand.isFlush || testedHand.isFullHouse ||
                                                testedHand.isTrips || testedHand.isQuads || testedHand.isStFlush))
    {
        return true;
    }

    // the player has raised 3 times the river, and is not a maniac player : he should have better than a set
    if (ctx.perPlayerContext.nbRaises > 2 && !(testedHand.isStraight || testedHand.isFlush || testedHand.isFullHouse ||
                                               testedHand.isQuads || testedHand.isStFlush))
    {
        return true;
    }

    return false;
}

bool HandPlausibilityChecker::isUnplausibleHandGivenRiverAllin(const PostFlopAnalysisFlags& testedHand,
                                                               CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.perPlayerContext.myHavePosition;
    auto& river = ctx.perPlayerContext.myStatistics.getRiverStatistics();

    if (ctx.commonContext.potOdd < 20)
    {
        return false;
    }

    if (river.getAgressionFactor() > 3 && river.getAgressionFrequency() > 50 &&
        river.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
    {
        return false; // he is usually very agressive, so don't make any guess
    }

    if (ctx.perPlayerContext.myIsInVeryLooseMode)
    {
        return false; // he is (temporarily ?) very agressive, so don't make any guess
    }

    if (!testedHand.usesFirst && !testedHand.usesSecond)
    {
        return true;
    }

    // the player has raised twice or more the river, and is not a maniac player : he should have at least a
    // straight
    if (ctx.perPlayerContext.nbRaises > 1 && !(testedHand.isStraight || testedHand.isFlush || testedHand.isFullHouse ||
                                               testedHand.isQuads || testedHand.isStFlush))
    {
        return true;
    }

    return false;
}

} // namespace pkt::core::player
