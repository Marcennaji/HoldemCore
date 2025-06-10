
#include "RangePlausibilityChecker.h"
#include <third_party/psim/psim.hpp>
#include "core/player/RangeManager.h"
#include "core/player/typedefs.h"

namespace pkt::core::player
{

bool RangePlausibilityChecker::isUnplausibleHandGivenFlopCheck(const PostFlopState& testedHand, CurrentHandContext& ctx)
{

    const int nbPlayers = ctx.nbPlayers;
    const bool bHavePosition = ctx.myHavePosition;
    auto& flop = ctx.myStatistics.getFlopStatistics();

    // the player is in position, he didn't bet on flop, he is not usually passive, and everybody checked on flop :

    if (bHavePosition &&
        !(flop.getAgressionFactor() < 2 && flop.getAgressionFrequency() < 30 &&
          flop.m_hands > MIN_HANDS_STATISTICS_ACCURATE) &&
        (testedHand.UsesFirst || testedHand.UsesSecond))
    {

        // woudn't slow play a medium hand on a dangerous board
        if (!testedHand.IsFullHousePossible &&
            ((testedHand.IsMiddlePair && !testedHand.IsFullHousePossible && ctx.nbRunningPlayers < 4) ||
             testedHand.IsTopPair || testedHand.IsOverPair ||
             (testedHand.IsTwoPair && !testedHand.IsFullHousePossible)) &&
            testedHand.IsFlushDrawPossible && testedHand.IsStraightDrawPossible)
            return true;

        // on a non-paired board, he would'nt slow play a straigth, a set or 2 pairs, if a flush draw is possible
        if (!testedHand.IsFullHousePossible && (testedHand.IsTrips || testedHand.IsStraight || testedHand.IsTwoPair) &&
            testedHand.IsFlushDrawPossible)
            return true;

        // wouldn't be passive with a decent hand, on position, if more than 1 opponent
        if (!testedHand.IsFullHousePossible &&
            (testedHand.IsTopPair || testedHand.IsOverPair || testedHand.IsTwoPair || testedHand.IsTrips) &&
            ctx.nbRunningPlayers > 2)
            return true;

        // on a paired board, he wouldn't check if he has a pocket overpair
        if (testedHand.IsFullHousePossible && testedHand.IsOverPair)
            return true;
    }
    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenFlopBet(const PostFlopState& testedHand, CurrentHandContext& ctx)
{

    const int nbPlayers = ctx.nbPlayers;
    const bool bHavePosition = ctx.myHavePosition;
    auto& flop = ctx.myStatistics.getFlopStatistics();

    if (flop.getAgressionFactor() > 3 && flop.getAgressionFrequency() > 50 &&
        flop.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is very agressive, so don't make any guess

    if (ctx.myIsInVeryLooseMode)
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!testedHand.UsesFirst && !testedHand.UsesSecond)
        return true;

    // the player made a donk bet on the flop, and is not a maniac player : he should have at least a middle or top pair
    // or a draw
    if (!bHavePosition && !ctx.myPreflopIsAggressor)
    {

        if (testedHand.IsOverCards || testedHand.StraightOuts >= 8 || testedHand.FlushOuts >= 8)
            return (ctx.nbRunningPlayers > 2 ? true : false);

        if (!((testedHand.IsTwoPair && !testedHand.IsFullHousePossible) || testedHand.IsStraight ||
              testedHand.IsFlush || testedHand.IsFullHouse || testedHand.IsTrips || testedHand.IsQuads ||
              testedHand.IsStFlush))
        {

            if (testedHand.IsNoPair)
                return true;

            if (testedHand.IsOnePair)
            {

                if (testedHand.IsFullHousePossible)
                    return true;

                if (!testedHand.IsMiddlePair && !testedHand.IsTopPair && !testedHand.IsOverPair)
                    return true;
            }
        }
    }

    // on a 3 or more players pot : if the player bets in position, he should have at least a middle pair
    if (bHavePosition && ctx.nbRunningPlayers > 2)
    {

        if (testedHand.IsOverCards || testedHand.StraightOuts >= 8 || testedHand.FlushOuts >= 8)
            return true;

        if (!((testedHand.IsTwoPair && !testedHand.IsFullHousePossible) || testedHand.IsStraight ||
              testedHand.IsFlush || testedHand.IsFullHouse || testedHand.IsTrips || testedHand.IsQuads ||
              testedHand.IsStFlush))
        {

            if (testedHand.IsNoPair)
                return true;

            if (testedHand.IsOnePair)
            {

                if (testedHand.IsFullHousePossible)
                    return true;

                if (!testedHand.IsMiddlePair && !testedHand.IsTopPair && !testedHand.IsOverPair)
                    return true;
            }
        }
    }

    // on a 3 or more players pot : if the player is first to act, and bets, he should have at least a top pair
    if (ctx.nbChecks == 0 && ctx.nbRunningPlayers > 2)
    {

        if (!((testedHand.IsTwoPair && !testedHand.IsFullHousePossible) || testedHand.IsStraight ||
              testedHand.IsFlush || testedHand.IsFullHouse || testedHand.IsTrips || testedHand.IsQuads ||
              testedHand.IsStFlush))
        {

            if (testedHand.IsNoPair)
                return true;

            if (testedHand.IsOnePair)
            {

                if (testedHand.IsFullHousePossible)
                    return true;

                if (!testedHand.IsTopPair && !testedHand.IsOverPair)
                    return true;
            }
        }
    }
    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenFlopCall(const PostFlopState& testedHand, CurrentHandContext& ctx)
{

    const int nbPlayers = ctx.nbPlayers;
    const bool bHavePosition = ctx.myHavePosition;
    auto& flop = ctx.myStatistics.getFlopStatistics();

    if (ctx.potOdd < 20)
        return false;

    if (ctx.myIsInVeryLooseMode)
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!testedHand.UsesFirst && !testedHand.UsesSecond)
        return true;

    if (ctx.flopBetsOrRaisesNumber > 0 && ctx.myCurrentHandActions.getFlopActions().back() == PLAYER_ACTION_CALL &&
        !(ctx.myStatistics.getWentToShowDown() > 35 &&
          ctx.myStatistics.getRiverStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE))
    {

        if (!((testedHand.IsTwoPair && !testedHand.IsFullHousePossible) || testedHand.IsStraight ||
              testedHand.IsFlush || testedHand.IsFullHouse || testedHand.IsTrips || testedHand.IsQuads ||
              testedHand.IsStFlush || testedHand.IsOverCards || testedHand.FlushOuts >= 8 ||
              testedHand.StraightOuts >= 8))
        {

            if (testedHand.IsNoPair)
                return true;

            if (ctx.flopBetsOrRaisesNumber > 1 && testedHand.IsOnePair && !testedHand.IsTopPair &&
                !testedHand.IsOverPair)
                return true;

            if (ctx.flopBetsOrRaisesNumber > 2 && (testedHand.IsOnePair || testedHand.IsOverCards))
                return true;

            if (ctx.nbRunningPlayers > 2 && testedHand.IsOnePair && !testedHand.IsTopPair && !testedHand.IsOverPair)
                return true;
        }
    }
    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenFlopRaise(const PostFlopState& testedHand, CurrentHandContext& ctx)
{

    const int nbPlayers = ctx.nbPlayers;
    const bool bHavePosition = ctx.myHavePosition;
    auto& flop = ctx.myStatistics.getFlopStatistics();

    if (flop.getAgressionFactor() > 3 && flop.getAgressionFrequency() > 50 &&
        flop.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (ctx.myIsInVeryLooseMode)
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!testedHand.UsesFirst && !testedHand.UsesSecond)
        return true;

    // the player has check-raised the flop, and is not a maniac player : he should have at least a top pair or a draw
    if (ctx.nbChecks == 1)
    {

        if ((testedHand.IsOverCards || testedHand.FlushOuts >= 8 || testedHand.StraightOuts >= 8) &&
            ctx.nbRunningPlayers > 2)
            return true;

        if (!((testedHand.IsTwoPair && !testedHand.IsFullHousePossible) || testedHand.IsStraight ||
              testedHand.IsFlush || testedHand.IsFullHouse || testedHand.IsTrips || testedHand.IsQuads ||
              testedHand.IsStFlush))
        {

            if (testedHand.IsNoPair)
                return true;

            if (testedHand.IsOnePair && !testedHand.IsFullHousePossible && !testedHand.IsTopPair &&
                !testedHand.IsOverPair)
                return true;
        }
    }

    // the player has raised or reraised the flop, and is not a maniac player : he should have at least a top pair
    if (ctx.nbRaises > 0)
    {

        if (!((testedHand.IsTwoPair && !testedHand.IsFullHousePossible) || testedHand.IsStraight ||
              testedHand.IsFlush || testedHand.IsFullHouse || testedHand.IsTrips || testedHand.IsQuads ||
              testedHand.IsStFlush))
        {

            if (testedHand.IsNoPair || (testedHand.IsOnePair && testedHand.IsFullHousePossible))
                return true;

            if (testedHand.IsOnePair && !testedHand.IsFullHousePossible &&
                !testedHand.IsTopPair & !testedHand.IsOverPair)
                return true;

            if (ctx.flopBetsOrRaisesNumber > 3 && (testedHand.IsOnePair))
                return true;

            if (ctx.flopBetsOrRaisesNumber > 4 && (testedHand.IsTwoPair))
                return true;
        }
    }

    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenFlopAllin(const PostFlopState& testedHand, CurrentHandContext& ctx)
{

    const int nbPlayers = ctx.nbPlayers;
    const bool bHavePosition = ctx.myHavePosition;
    auto& flop = ctx.myStatistics.getFlopStatistics();

    if (ctx.potOdd < 20)
        return false;

    if (flop.getAgressionFactor() > 3 && flop.getAgressionFrequency() > 50 &&
        flop.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (ctx.myIsInVeryLooseMode)
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!testedHand.UsesFirst && !testedHand.UsesSecond)
        return true;

    if (!((testedHand.IsTwoPair && !testedHand.IsFullHousePossible) || testedHand.IsStraight || testedHand.IsFlush ||
          testedHand.IsFullHouse || testedHand.IsTrips || testedHand.IsQuads || testedHand.IsStFlush))
    {

        if (testedHand.IsNoPair || (testedHand.IsOnePair && testedHand.IsFullHousePossible))
            return true;

        if (testedHand.IsOnePair && !testedHand.IsFullHousePossible && !testedHand.IsTopPair & !testedHand.IsOverPair)
            return true;

        if (ctx.flopBetsOrRaisesNumber > 3 && (testedHand.IsOnePair))
            return true;

        if (ctx.flopBetsOrRaisesNumber > 4 && (testedHand.IsTwoPair))
            return true;
    }
    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenTurnCheck(const PostFlopState& testedHand, CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.myHavePosition;
    auto& turn = ctx.myStatistics.getTurnStatistics();

    // the player is in position, he isn't usually passive, and everybody checked
    if (bHavePosition && !(turn.getAgressionFactor() < 2 && turn.getAgressionFrequency() < 30 &&
                           turn.m_hands > MIN_HANDS_STATISTICS_ACCURATE))
    {

        if (testedHand.IsPocketPair && testedHand.IsOverPair)
            return true;

        // woudn't slow play a medium hand on a dangerous board, if there was no action on flop
        if (((testedHand.UsesFirst || testedHand.UsesSecond) && ctx.flopBetsOrRaisesNumber == 0 &&
                 testedHand.IsTopPair ||
             (testedHand.IsTwoPair && !testedHand.IsFullHousePossible) || testedHand.IsTrips) &&
            testedHand.IsFlushDrawPossible)
            return true;

        // wouldn't be passive with a decent hand, on position, if more than 1 opponent
        if (((testedHand.UsesFirst || testedHand.UsesSecond) &&
             ((testedHand.IsTwoPair && !testedHand.IsFullHousePossible) || testedHand.IsTrips)) &&
            ctx.nbRunningPlayers > 2)
            return true;
    }

    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenTurnBet(const PostFlopState& testedHand, CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.myHavePosition;
    auto& turn = ctx.myStatistics.getTurnStatistics();

    if (turn.getAgressionFactor() > 3 && turn.getAgressionFrequency() > 50 &&
        turn.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (ctx.myIsInVeryLooseMode)
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!testedHand.UsesFirst && !testedHand.UsesSecond)
        return true;

    // the player made a donk bet on turn, and is not a maniac player : he should have at least a top pair
    if (!bHavePosition && !ctx.myFlopIsAggressor && ctx.flopBetsOrRaisesNumber > 0)
    {

        if ((testedHand.IsOverCards || testedHand.FlushOuts >= 8 || testedHand.StraightOuts >= 8) &&
            ctx.nbRunningPlayers > 2)
            return true;

        if (!((testedHand.IsTwoPair && !testedHand.IsFullHousePossible) || testedHand.IsStraight ||
              testedHand.IsFlush || testedHand.IsFullHouse || testedHand.IsTrips || testedHand.IsQuads ||
              testedHand.IsStFlush))
        {

            if (testedHand.IsNoPair)
                return true;

            if (testedHand.IsOnePair && !testedHand.IsTopPair && !testedHand.IsOverPair &&
                !testedHand.IsFullHousePossible)
                return true;
        }
    }

    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenTurnCall(const PostFlopState& testedHand, CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.myHavePosition;
    auto& turn = ctx.myStatistics.getTurnStatistics();

    if (ctx.potOdd < 20)
        return false;

    if (ctx.myIsInVeryLooseMode)
        return false; // he is (temporarily ?) very loose, so don't make any guess

    if (!testedHand.UsesFirst && !testedHand.UsesSecond)
        return true;

    // the player called a bet on flop and turn, and he is not loose
    if (ctx.turnBetsOrRaisesNumber > 0 && ctx.flopBetsOrRaisesNumber > 0 &&
        ctx.myCurrentHandActions.getTurnActions().back() && !ctx.myFlopIsAggressor &&
        !(ctx.myStatistics.getWentToShowDown() > 30 &&
          ctx.myStatistics.getRiverStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE))
    {

        if (!((testedHand.IsTwoPair && !testedHand.IsFullHousePossible) || testedHand.IsStraight ||
              testedHand.IsFlush || testedHand.IsFullHouse || testedHand.IsTrips || testedHand.IsQuads ||
              testedHand.IsStFlush || testedHand.FlushOuts >= 8 || testedHand.StraightOuts >= 8))
        {

            if (testedHand.IsNoPair || (testedHand.IsOnePair && testedHand.IsFullHousePossible))
                return true;

            if (testedHand.IsOnePair && !testedHand.IsTopPair && !testedHand.IsOverPair &&
                !testedHand.IsFullHousePossible)
                return true;

            if (ctx.turnBetsOrRaisesNumber > 2 && testedHand.IsOnePair)
                return true;
        }
    }
    // the player called a raise on turn, and is not loose : he has at least a top pair or a good draw
    if (ctx.turnBetsOrRaisesNumber > 1 && ctx.myCurrentHandActions.getTurnActions().back() == PLAYER_ACTION_CALL &&
        !(ctx.myStatistics.getWentToShowDown() > 35 &&
          ctx.myStatistics.getRiverStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE))
    {

        if (!(((testedHand.IsTwoPair && !testedHand.IsFullHousePossible) && !testedHand.IsFullHousePossible) ||
              testedHand.IsStraight || testedHand.IsFlush || testedHand.IsFullHouse || testedHand.IsTrips ||
              testedHand.IsQuads || testedHand.IsStFlush || testedHand.IsOverCards || testedHand.IsFlushDrawPossible ||
              testedHand.FlushOuts >= 8 || testedHand.StraightOuts >= 8))
        {

            if (testedHand.IsNoPair || (testedHand.IsOnePair && testedHand.IsFullHousePossible))
                return true;

            if (testedHand.IsOnePair && !testedHand.IsTopPair && !testedHand.IsFullHousePossible &&
                !testedHand.IsOverPair)
                return true;
        }
    }

    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenTurnRaise(const PostFlopState& testedHand, CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.myHavePosition;
    auto& turn = ctx.myStatistics.getTurnStatistics();

    if (turn.getAgressionFactor() > 3 && turn.getAgressionFrequency() > 50 &&
        turn.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is very agressive, so don't make any guess

    if (ctx.myIsInVeryLooseMode)
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!testedHand.UsesFirst && !testedHand.UsesSecond)
        return true;

    // if nobody has bet the flop, he should at least have a top pair
    if (ctx.flopBetsOrRaisesNumber == 0)
    {

        if (testedHand.IsTopPair || testedHand.IsOverPair ||
            (testedHand.IsTwoPair && !testedHand.IsFullHousePossible) || testedHand.IsStraight || testedHand.IsFlush ||
            testedHand.IsFullHouse || testedHand.IsTrips || testedHand.IsQuads || testedHand.IsStFlush)
            return false;
        else
            return true;
    }
    // if he was not the agressor on flop, and an other player has bet the flop, then he should have at least a top pair
    if (!ctx.myFlopIsAggressor && ctx.flopBetsOrRaisesNumber > 0)
    {

        if (testedHand.IsTopPair || testedHand.IsOverPair ||
            (testedHand.IsTwoPair && !testedHand.IsFullHousePossible) || testedHand.IsStraight || testedHand.IsFlush ||
            testedHand.IsFullHouse || testedHand.IsTrips || testedHand.IsQuads || testedHand.IsStFlush)
            return false;
        else
            return true;
    }
    // the player has raised twice the turn, and is not a maniac player : he should have at least two pairs
    if (ctx.nbRaises == 2 &&
        !((testedHand.IsTwoPair && !testedHand.IsFullHousePossible) || testedHand.IsStraight || testedHand.IsFlush ||
          testedHand.IsFullHouse || testedHand.IsTrips || testedHand.IsQuads || testedHand.IsStFlush))
        return true;

    // the player has raised 3 times the turn, and is not a maniac player : he should have better than a set
    if (ctx.nbRaises > 2 && !(testedHand.IsStraight || testedHand.IsFlush || testedHand.IsFullHouse ||
                              testedHand.IsQuads || testedHand.IsStFlush))
        return true;

    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenTurnAllin(const PostFlopState& testedHand, CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.myHavePosition;
    auto& turn = ctx.myStatistics.getTurnStatistics();

    if (ctx.potOdd < 20)
        return false;

    if (turn.getAgressionFactor() > 3 && turn.getAgressionFrequency() > 50 &&
        turn.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (ctx.myIsInVeryLooseMode)
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!testedHand.UsesFirst && !testedHand.UsesSecond)
        return true;

    // the player has raised twice the turn, and is not a maniac player : he should have at least two pairs
    if (ctx.nbRaises == 2 &&
        !((testedHand.IsTwoPair && !testedHand.IsFullHousePossible) || testedHand.IsStraight || testedHand.IsFlush ||
          testedHand.IsFullHouse || testedHand.IsTrips || testedHand.IsQuads || testedHand.IsStFlush))
        return true;

    // the player has raised 3 times the turn, and is not a maniac player : he should have better than a set
    if (ctx.nbRaises > 2 && !(testedHand.IsStraight || testedHand.IsFlush || testedHand.IsFullHouse ||
                              testedHand.IsQuads || testedHand.IsStFlush))
        return true;

    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenRiverCheck(const PostFlopState& testedHand,
                                                                CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.myHavePosition;
    auto& river = ctx.myStatistics.getRiverStatistics();

    // todo

    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenRiverBet(const PostFlopState& testedHand, CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.myHavePosition;
    auto& river = ctx.myStatistics.getRiverStatistics();

    if (river.getAgressionFactor() > 3 && river.getAgressionFrequency() > 50 &&
        river.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (ctx.myIsInVeryLooseMode)
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!testedHand.UsesFirst && !testedHand.UsesSecond)
        return true;

    // the player has bet the river, was not the agressor on turn and river, and is not a maniac player : he should
    // have at least 2 pairs
    if (ctx.flopBetsOrRaisesNumber > 1 && !ctx.myFlopIsAggressor && ctx.turnBetsOrRaisesNumber > 1 &&
        !ctx.myTurnIsAggressor)
    {

        if ((testedHand.IsTwoPair && !testedHand.IsFullHousePossible) || testedHand.IsStraight || testedHand.IsFlush ||
            testedHand.IsFullHouse || testedHand.IsTrips || testedHand.IsQuads || testedHand.IsStFlush)
            return false;
        else
            return true;
    }

    // the player has bet the river, is out of position on a multi-players pot, in a hand with some action, and is
    // not a maniac player : he should have at least 2 pairs
    if (!bHavePosition && ctx.nbRunningPlayers > 2 &&
        ((ctx.flopBetsOrRaisesNumber > 1 && !ctx.myFlopIsAggressor) ||
         (ctx.turnBetsOrRaisesNumber > 1 && !ctx.myTurnIsAggressor)))
    {

        if ((testedHand.IsTwoPair && !testedHand.IsFullHousePossible) || testedHand.IsStraight || testedHand.IsFlush ||
            testedHand.IsFullHouse || testedHand.IsTrips || testedHand.IsQuads || testedHand.IsStFlush)
            return false;
        else
            return true;
    }
    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenRiverCall(const PostFlopState& testedHand, CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.myHavePosition;
    auto& river = ctx.myStatistics.getRiverStatistics();

    if (ctx.potOdd < 20)
        return false;

    if (river.getAgressionFactor() > 3 && river.getAgressionFrequency() > 50 &&
        river.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (ctx.myIsInVeryLooseMode)
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!testedHand.UsesFirst && !testedHand.UsesSecond)
        return true;

    // the player has called the river on a multi-players pot, and is not a loose player : he should have at least a
    // top pair
    if (ctx.nbRunningPlayers > 2)
    {

        if (!((testedHand.IsTwoPair && !testedHand.IsFullHousePossible) || testedHand.IsStraight ||
              testedHand.IsFlush || testedHand.IsFullHouse || testedHand.IsTrips || testedHand.IsQuads ||
              testedHand.IsStFlush))
        {

            if (testedHand.IsNoPair || (testedHand.IsOnePair && testedHand.IsFullHousePossible))
                return true;

            if (testedHand.IsOnePair && !testedHand.IsTopPair & !testedHand.IsOverPair)
                return true;
        }
    }

    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenRiverRaise(const PostFlopState& testedHand,
                                                                CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.myHavePosition;
    auto& river = ctx.myStatistics.getRiverStatistics();

    if (river.getAgressionFactor() > 3 && river.getAgressionFrequency() > 50 &&
        river.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (ctx.myIsInVeryLooseMode)
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    // the player has raised the river, and is not a maniac player : he should have at least 2 pairs
    if (!testedHand.UsesFirst && !testedHand.UsesSecond)
        return true;

    if (testedHand.IsStraight || testedHand.IsFlush || testedHand.IsFullHouse || testedHand.IsTrips ||
        testedHand.IsQuads || testedHand.IsStFlush || (testedHand.IsTwoPair && !testedHand.IsFullHousePossible))
        return false;
    else
        return true;

    // the player has raised the river, is out of position on a multi-players pot, in a hand with some action, and
    // is not a maniac player : he should have at least a set
    if (!bHavePosition && ctx.nbRunningPlayers > 2 &&
        ((ctx.flopBetsOrRaisesNumber > 1 && !ctx.myFlopIsAggressor) ||
         (ctx.turnBetsOrRaisesNumber > 1 && !ctx.myTurnIsAggressor)))
    {

        if (testedHand.IsStraight || testedHand.IsFlush || testedHand.IsFullHouse || testedHand.IsTrips ||
            testedHand.IsQuads || testedHand.IsStFlush)
            return false;
        else
            return true;
    }

    // the player has raised twice the river, and is not a maniac player : he should have at least trips
    if (ctx.nbRaises == 2 && !(testedHand.IsStraight || testedHand.IsFlush || testedHand.IsFullHouse ||
                               testedHand.IsTrips || testedHand.IsQuads || testedHand.IsStFlush))
        return true;

    // the player has raised 3 times the river, and is not a maniac player : he should have better than a set
    if (ctx.nbRaises > 2 && !(testedHand.IsStraight || testedHand.IsFlush || testedHand.IsFullHouse ||
                              testedHand.IsQuads || testedHand.IsStFlush))
        return true;

    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenRiverAllin(const PostFlopState& testedHand,
                                                                CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.myHavePosition;
    auto& river = ctx.myStatistics.getRiverStatistics();

    if (ctx.potOdd < 20)
        return false;

    if (river.getAgressionFactor() > 3 && river.getAgressionFrequency() > 50 &&
        river.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (ctx.myIsInVeryLooseMode)
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!testedHand.UsesFirst && !testedHand.UsesSecond)
        return true;

    // the player has raised twice or more the river, and is not a maniac player : he should have at least a
    // straight
    if (ctx.nbRaises > 1 && !(testedHand.IsStraight || testedHand.IsFlush || testedHand.IsFullHouse ||
                              testedHand.IsQuads || testedHand.IsStFlush))
        return true;

    return false;
}

} // namespace pkt::core::player
