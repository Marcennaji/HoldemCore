#include "FlopPlausibilityChecker.h"
#include "PlausibilityHelpers.h"
#include "core/player/range/RangeEstimator.h"
#include "core/player/typedefs.h"

namespace pkt::core::player
{

bool FlopPlausibilityChecker::isUnplausibleHandGivenFlopCheck(const PostFlopAnalysisFlags& testedHand,
                                                              const CurrentHandContext& ctx)
{

    const bool bHavePosition = ctx.personalContext.hasPosition;
    auto& flop = ctx.personalContext.statistics.flopStatistics;

    float agressionFactor = flop.getAgressionFactor();
    float agressionFrequency = flop.getAgressionFrequency();

    // the player is in position, he didn't bet on flop, he is not usually passive, and everybody checked on flop :

    if (bHavePosition &&
        !PlausibilityHelpers::isPassivePlayer(ctx) &&
        (testedHand.usesFirst || testedHand.usesSecond))
    {

        // woudn't slow play a medium hand on a dangerous board
        if (PlausibilityHelpers::isUnpairedBoard(testedHand) &&
            ((testedHand.isMiddlePair && !testedHand.isFullHousePossible &&
              ctx.commonContext.playersContext.actingPlayersList->size() < 4) ||
             testedHand.isTopPair || testedHand.isOverPair ||
             (testedHand.isTwoPair && !testedHand.isFullHousePossible)) &&
            PlausibilityHelpers::isVeryDangerousBoard(testedHand))
        {
            return true;
        }

        // on a non-paired board, he would'nt slow play a straigth, a set or 2 pairs, if a flush draw is possible
        if (PlausibilityHelpers::isUnpairedBoard(testedHand) && (testedHand.isTrips || testedHand.isStraight || testedHand.isTwoPair) &&
            testedHand.isFlushDrawPossible)
        {
            return true;
        }

        // wouldn't be passive with a decent hand, on position, if more than 1 opponent
        if (PlausibilityHelpers::isUnpairedBoard(testedHand) &&
            PlausibilityHelpers::hasDecentMadeHand(testedHand) &&
            ctx.commonContext.playersContext.actingPlayersList->size() > 2)
        {
            return true;
        }

        // on a paired board, he wouldn't check if he has a pocket overpair
        if (PlausibilityHelpers::isPairedBoard(testedHand) && testedHand.isOverPair)
        {
            return true;
        }
    }
    return false;
}

bool FlopPlausibilityChecker::isUnplausibleHandGivenFlopBet(const PostFlopAnalysisFlags& testedHand,
                                                            const CurrentHandContext& ctx)
{
    // Maniac players are unpredictable
    if (PlausibilityHelpers::isManiacPlayer(ctx))
    {
        return false;
    }

    // Hand not using hole cards is implausible for betting
    if (PlausibilityHelpers::handDoesNotUseHoleCards(testedHand))
    {
        return true;
    }

    // Donk bet analysis: player made a bet out of position without being preflop aggressor
    if (PlausibilityHelpers::isDonkBet(ctx))
    {
        // Overcards or strong draws in multiway are questionable for donk betting
        if (testedHand.isOverCards || PlausibilityHelpers::hasStrongDraw(testedHand))
        {
            return PlausibilityHelpers::isMultiwayPot(ctx);
        }

        // Strong hands can donk bet
        if (PlausibilityHelpers::hasStrongHand(testedHand))
        {
            return false;
        }

        // Weak hands (no pair, weak pairs, pairs on paired board) shouldn't donk bet
        if (PlausibilityHelpers::hasVeryWeakHand(testedHand) ||
            (testedHand.isOnePair && !testedHand.isMiddlePair && !testedHand.isTopPair && !testedHand.isOverPair))
        {
            return true;
        }
    }

    // In position multiway: need at least middle pair
    if (PlausibilityHelpers::isInPosition(ctx) && PlausibilityHelpers::isMultiwayPot(ctx))
    {
        // Overcards or draws are questionable in multiway
        if (testedHand.isOverCards || PlausibilityHelpers::hasStrongDraw(testedHand))
        {
            return true;
        }

        // Strong hands are always good
        if (PlausibilityHelpers::hasStrongHand(testedHand))
        {
            return false;
        }

        // No pair is bad in multiway
        if (testedHand.isNoPair)
        {
            return true;
        }

        // One pair analysis
        if (testedHand.isOnePair)
        {
            // Pairs on paired board are questionable
            if (PlausibilityHelpers::isPairedBoard(testedHand))
            {
                return true;
            }
            // Middle pair or better is acceptable
            return !(testedHand.isMiddlePair || testedHand.isTopPair || testedHand.isOverPair);
        }
    }

    // First to act in multiway: need at least top pair
    if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber == 0 && PlausibilityHelpers::isMultiwayPot(ctx))
    {
        // Strong hands are always acceptable
        if (PlausibilityHelpers::hasStrongHand(testedHand))
        {
            return false;
        }

        // No pair is unacceptable
        if (testedHand.isNoPair)
        {
            return true;
        }

        // One pair analysis: need top pair or overpair
        if (testedHand.isOnePair)
        {
            // Pairs on paired board are questionable
            if (PlausibilityHelpers::isPairedBoard(testedHand))
            {
                return true;
            }
            // Need at least top pair
            return !(testedHand.isTopPair || testedHand.isOverPair);
        }
    }
    return false;
}

bool FlopPlausibilityChecker::isUnplausibleHandGivenFlopCall(const PostFlopAnalysisFlags& testedHand,
                                                             const CurrentHandContext& ctx)
{
    // Good pot odds make calls more reasonable
    if (ctx.commonContext.bettingContext.potOdd < 20)
    {
        return false;
    }

    // Very loose mode players are unpredictable
    if (ctx.personalContext.actions.isInVeryLooseMode)
    {
        return false;
    }

    // Hand not using hole cards shouldn't call
    if (PlausibilityHelpers::handDoesNotUseHoleCards(testedHand))
    {
        return true;
    }

    // Analysis for players who called with action on flop
    if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber > 0 &&
        ctx.personalContext.actions.currentHandActions.getActions(GameState::Flop).back().type == ActionType::Call &&
        !(ctx.personalContext.statistics.getWentToShowDown() > 35 &&
          ctx.personalContext.statistics.riverStatistics.hands > MIN_HANDS_STATISTICS_ACCURATE))
    {
        // Strong hands and draws are acceptable for calling
        if (PlausibilityHelpers::hasStrongHand(testedHand) || testedHand.isOverCards ||
            PlausibilityHelpers::hasStrongDraw(testedHand))
        {
            return false;
        }

        // No pair is generally bad for calling
        if (testedHand.isNoPair)
        {
            return true;
        }

        // Multiple bets make weak pairs implausible
        if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber > 1 && testedHand.isOnePair &&
            !testedHand.isTopPair && !testedHand.isOverPair)
        {
            return true;
        }

        // Heavy action makes most hands implausible
        if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber > 2 &&
            (testedHand.isOnePair || testedHand.isOverCards))
        {
            return true;
        }

        // Multiway with weak pair is implausible
        if (PlausibilityHelpers::isMultiwayPot(ctx) && testedHand.isOnePair && !testedHand.isTopPair &&
            !testedHand.isOverPair)
        {
            return true;
        }
    }
    return false;
}

bool FlopPlausibilityChecker::isUnplausibleHandGivenFlopRaise(const PostFlopAnalysisFlags& testedHand,
                                                              const CurrentHandContext& ctx)
{
    // Maniac players are unpredictable
    if (PlausibilityHelpers::isManiacPlayer(ctx))
    {
        return false;
    }

    // Hand not using hole cards shouldn't raise
    if (PlausibilityHelpers::handDoesNotUseHoleCards(testedHand))
    {
        return true;
    }

    // Check-raise analysis: should have strong hand or draw
    if (ctx.personalContext.actions.currentHandActions.getActionsNumber(GameState::Flop, ActionType::Check) == 1)
    {
        // Multiway check-raise with draws is questionable
        if ((testedHand.isOverCards || PlausibilityHelpers::hasStrongDraw(testedHand)) &&
            PlausibilityHelpers::isMultiwayPot(ctx))
        {
            return true;
        }

        // Strong hands can check-raise
        if (PlausibilityHelpers::hasStrongHand(testedHand))
        {
            return false;
        }

        // Weak hands shouldn't check-raise
        if (PlausibilityHelpers::hasWeakUnpairedHand(testedHand))
        {
            return true;
        }
    }

    // Regular raise/reraise analysis: need strong hands
    if (ctx.personalContext.actions.currentHandActions.getActionsNumber(GameState::Flop, ActionType::Raise) > 0)
    {
        // Strong hands can raise
        if (PlausibilityHelpers::hasStrongHand(testedHand))
        {
            return false;
        }

        // Very weak hands shouldn't raise
        if (PlausibilityHelpers::hasVeryWeakHand(testedHand) ||
            (testedHand.isOnePair && !testedHand.isTopPair && !testedHand.isOverPair))
        {
            return true;
        }

        // Heavy action makes weaker hands implausible
        if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber > 3 && testedHand.isOnePair)
        {
            return true;
        }

        if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber > 4 && testedHand.isTwoPair)
        {
            return true;
        }
    }

    return false;
}

bool FlopPlausibilityChecker::isUnplausibleHandGivenFlopAllin(const PostFlopAnalysisFlags& testedHand,
                                                              const CurrentHandContext& ctx)
{
    // Good pot odds make all-ins more reasonable
    if (ctx.commonContext.bettingContext.potOdd < 20)
    {
        return false;
    }

    // Maniac players are unpredictable
    if (PlausibilityHelpers::isManiacPlayer(ctx))
    {
        return false;
    }

    // Hand not using hole cards shouldn't go all-in
    if (PlausibilityHelpers::handDoesNotUseHoleCards(testedHand))
    {
        return true;
    }

    // Strong hands can go all-in
    if (PlausibilityHelpers::hasStrongHand(testedHand))
    {
        return false;
    }

    // Very weak hands shouldn't go all-in
    if (PlausibilityHelpers::hasVeryWeakHand(testedHand) ||
        (testedHand.isOnePair && !testedHand.isTopPair && !testedHand.isOverPair))
    {
        return true;
    }

    // Heavy action makes weaker hands implausible
    if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber > 3 && testedHand.isOnePair)
    {
        return true;
    }

    if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber > 4 && testedHand.isTwoPair)
    {
        return true;
    }
    return false;
}

} // namespace pkt::core::player
