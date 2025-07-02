// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "PreflopRangeEstimator.h"
#include <cmath>
#include <core/engine/model/Ranges.h>
#include <core/interfaces/IHand.h>
#include <core/player/range/RangeRefiner.h>
#include <core/player/strategy/CurrentHandContext.h>
#include <core/services/GlobalServices.h>
#include <string>
#include <vector>

namespace pkt::core::player
{
using namespace std;

PreflopRangeEstimator::PreflopRangeEstimator(IHand* hand, int playerId) : myHand(hand), myPlayerId(playerId)
{
}

string PreflopRangeEstimator::computeEstimatedPreflopRange(CurrentHandContext& ctx)
{
    string estimatedRange;

    GlobalServices::instance().logger()->verbose("\n\testimated range for " + std::to_string(ctx.myID) + " : ");

    PreflopStatistics preflop = ctx.myStatistics.getPreflopStatistics();
    const int nbPlayers = ctx.nbPlayers;

    GlobalServices::instance().logger()->verbose(
        "  " + std::to_string(preflop.getVoluntaryPutMoneyInPot()) + "/" + std::to_string(preflop.getPreflopRaise()) +
        ", 3B: " + std::to_string(preflop.getPreflop3Bet()) + ", 4B: " + std::to_string(preflop.getPreflop4Bet()) +
        ", C3B: " + std::to_string(preflop.getPreflopCall3BetsFrequency()) +
        ", pot odd: " + std::to_string(ctx.potOdd) + " " + "\n\t\t");

    // if the player was BB and has checked preflop, then he can have anything, except his approximative BB raising
    // range
    if (myHand->getPreflopRaisesNumber() == 0 && ctx.myPosition == BB)
    {
        const float raiseFactor = (preflop.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
                                      ? preflop.getPreflopRaise() * 0.8
                                      : RangeEstimator::getStandardRaisingRange(nbPlayers) * 0.8;

        const string excludeRange = RangeEstimator::getStringRange(nbPlayers, raiseFactor);
        GlobalServices::instance().logger()->verbose("any cards except " + excludeRange + '\n');
        estimatedRange = RangeRefiner::deduceRange(ANY_CARDS_RANGE, excludeRange);
    }
    else
    {
        // if the player is the last raiser :
        if (ctx.preflopLastRaiser && myPlayerId == ctx.preflopLastRaiser->getId())
        {
            estimatedRange = computeEstimatedPreflopRangeFromLastRaiser(ctx);
        }
        else
        {
            // if the player is the last caller :
            estimatedRange = computeEstimatedPreflopRangeFromCaller(ctx);
        }
    }

    return estimatedRange;
}

std::string PreflopRangeEstimator::computeEstimatedPreflopRangeFromLastRaiser(CurrentHandContext& ctx) const
{
    const int nbPlayers = ctx.nbPlayers;

    float range = 0;

    GlobalServices::instance().logger()->verbose(" [ player is last raiser ] ");
    PreflopStatistics preflopOpponent = ctx.preflopLastRaiser->getStatistics(nbPlayers).getPreflopStatistics();

    if (ctx.myStatistics.getPreflopStatistics().m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
    {
        if (myHand->getPreflopRaisesNumber() == 1)
        {
            range = preflopOpponent.getPreflopRaise();
        }
        else
        {
            // Assume the player has adapted their raising range to the previous raiser
            if (preflopOpponent.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
            {
                if (myHand->getPreflopRaisesNumber() == 2)
                {
                    range = preflopOpponent.getPreflopRaise() * 0.7;
                }
                else if (myHand->getPreflopRaisesNumber() == 3)
                {
                    range = preflopOpponent.getPreflop3Bet() * 0.7;
                }
                else if (myHand->getPreflopRaisesNumber() > 3)
                {
                    range = preflopOpponent.getPreflop4Bet() / (myHand->getPreflopRaisesNumber() / 2);
                }
            }
            else
            {
                if (myHand->getPreflopRaisesNumber() == 2)
                {
                    range = preflopOpponent.getPreflopRaise();
                }
                else if (myHand->getPreflopRaisesNumber() == 3)
                {
                    range = preflopOpponent.getPreflop3Bet();
                }
                else if (myHand->getPreflopRaisesNumber() > 3)
                {
                    range = preflopOpponent.getPreflop4Bet() / (myHand->getPreflopRaisesNumber() / 2);
                }
            }
        }
    }
    else
    {
        range = RangeEstimator::getStandardRaisingRange(nbPlayers);

        GlobalServices::instance().logger()->verbose(", but not enough hands -> getting the standard range : " +
                                                     std::to_string(range));

        if (myHand->getPreflopRaisesNumber() == 2)
        {
            range = range * 0.3;
        }
        else if (myHand->getPreflopRaisesNumber() == 3)
        {
            range = range * 0.2;
        }
        else if (myHand->getPreflopRaisesNumber() > 3)
        {
            range = range * 0.1;
        }
    }

    GlobalServices::instance().logger()->verbose("range is " + std::to_string(range));

    if (nbPlayers > 3)
    {
        // Adjust range based on position
        if (ctx.preflopLastRaiser->getPosition() == UTG || ctx.preflopLastRaiser->getPosition() == UtgPlusOne ||
            ctx.preflopLastRaiser->getPosition() == UtgPlusTwo)
        {
            range = range * 0.9;
        }
        else if (ctx.preflopLastRaiser->getPosition() == BUTTON || ctx.preflopLastRaiser->getPosition() == CUTOFF)
        {
            range = range * 1.5;
        }

        GlobalServices::instance().logger()->verbose(", position adjusted range is " + std::to_string(range));
    }

    // Adjust range for loose/aggressive mode
    if (ctx.preflopLastRaiser->isInVeryLooseMode(nbPlayers))
    {
        if (range < 40)
        {
            range = 40;
            GlobalServices::instance().logger()->verbose("\t\toveragression detected, setting range to " +
                                                         std::to_string(range));
        }
    }

    // Add error margin
    range++;

    range = ceil(range);

    if (range < 1)
    {
        range = 1;
    }

    if (range > 100)
    {
        range = 100;
    }

    GlobalServices::instance().logger()->verbose("\n\t\testimated range is " + std::to_string(range) + " % ");

    return RangeEstimator::getStringRange(nbPlayers, range);
}

std::string PreflopRangeEstimator::computeEstimatedPreflopRangeFromCaller(CurrentHandContext& ctx) const
{
    // The player is not the last raiser but has called a raise or limped in preflop

    bool isTopRange = true;
    std::vector<std::string> ranges;
    std::vector<float> rangesValues;
    const int nbPlayers = myHand->getSeatsList()->size();
    float range = 0;

    // Step 1: Analyze player actions
    int raises = 0, calls = 0;
    analyzePlayerActions(ctx, raises, calls);

    float estimatedStartingRange = calculateStartingRange(ctx, nbPlayers);

    range = adjustRangeForPosition(ctx, nbPlayers, estimatedStartingRange);

    range = adjustRangeForPotOdds(ctx, range);

    range = adjustRangeForPreflopRaises(ctx, raises, range);

    range = handleLooseAggressiveOpponents(ctx, raises, range);

    range = finalizeRange(range);

    std::string rangeString = finalizeEstimatedRange(ctx, ranges, rangesValues, range, nbPlayers, isTopRange);

    GlobalServices::instance().logger()->verbose("Estimated range for player " + std::to_string(ctx.myID) + " is {" +
                                                 rangeString + "}");
    return rangeString;
}
void PreflopRangeEstimator::analyzePlayerActions(const CurrentHandContext& ctx, int& raises, int& calls) const
{
    for (const auto& action : ctx.myCurrentHandActions.getPreflopActions())
    {
        if (action == PlayerActionRaise || action == PlayerActionAllin)
        {
            raises++;
        }
        else if (action == PlayerActionCall)
        {
            calls++;
        }
    }
}
float PreflopRangeEstimator::calculateStartingRange(const CurrentHandContext& ctx, int nbPlayers) const
{
    float estimatedStartingRange = ctx.myStatistics.getPreflopStatistics().getVoluntaryPutMoneyInPot();

    if (ctx.myStatistics.getPreflopStatistics().m_hands < MIN_HANDS_STATISTICS_ACCURATE)
    {
        estimatedStartingRange = RangeEstimator::getStandardCallingRange(nbPlayers);
        GlobalServices::instance().logger()->verbose(" [ not enough hands, getting the standard calling range ] ");
    }

    GlobalServices::instance().logger()->verbose(" estimated starting range : " +
                                                 std::to_string(estimatedStartingRange));
    return estimatedStartingRange;
}
float PreflopRangeEstimator::adjustRangeForPosition(const CurrentHandContext& ctx, int nbPlayers,
                                                    const float currentRange) const
{
    float range = currentRange;

    if (nbPlayers > 3)
    {
        if (ctx.myPosition == UTG || ctx.myPosition == UtgPlusOne || ctx.myPosition == UtgPlusTwo)
        {
            range *= 0.9;
        }
        else if (ctx.myPosition == BUTTON || ctx.myPosition == CUTOFF)
        {
            range *= 1.4;
        }
    }

    if (range > 100)
    {
        range = 100;
    }

    GlobalServices::instance().logger()->verbose("Position adjusted starting range : " + std::to_string(range));
    return range;
}
float PreflopRangeEstimator::adjustRangeForPotOdds(const CurrentHandContext& ctx, const float currentRange) const
{
    const int potOdd = ctx.potOdd;
    float range = currentRange;

    if (potOdd > 70 && potOdd < 85)
    {
        range *= 0.7;
    }
    else if (potOdd >= 85 && potOdd < 95)
    {
        range *= 0.5;
    }
    else if (potOdd >= 95 && potOdd < 99)
    {
        range *= 0.3;
    }
    else if (potOdd >= 99)
    {
        range *= 0.1;
    }
    else if (potOdd <= 20 && myHand->getPreflopRaisesNumber() < 2)
    {
        range = 40;
    }

    GlobalServices::instance().logger()->verbose("Pot odd adjusted starting range : " + std::to_string(range));
    return range;
}
float PreflopRangeEstimator::adjustRangeForPreflopRaises(const CurrentHandContext& ctx, int opponentRaises,
                                                         const float currentRange) const
{
    float range = currentRange;
    if (myHand->getPreflopRaisesNumber() == 0)
    {
        range = handleLimpRange(ctx, range);
    }
    else if (myHand->getPreflopRaisesNumber() == 1)
    {
        range = handleSingleRaiseRange(ctx, range);
    }
    else if (myHand->getPreflopRaisesNumber() == 2)
    {
        range = handleThreeBetRange(ctx, opponentRaises, range);
    }
    else if (myHand->getPreflopRaisesNumber() > 2)
    {
        range = handleFourBetOrMoreRange(ctx, opponentRaises, range);
    }
    return range;
}
float PreflopRangeEstimator::handleLooseAggressiveOpponents(const CurrentHandContext& ctx, int opponentRaises,
                                                            const float currentRange) const
{
    const int lastRaiserId = myHand->getPreflopLastRaiserId();
    float range = currentRange;

    if (opponentRaises > 0 && lastRaiserId != -1 && myHand->getPreflopRaisesNumber() == 1)
    {
        if (ctx.preflopLastRaiser->isInVeryLooseMode(ctx.nbPlayers))
        {
            if (ctx.nbPlayers > 6 && range < 20)
            {
                range = 20;
            }
            else if (ctx.nbPlayers > 4 && range < 30)
            {
                range = 30;
            }
            else if (ctx.nbPlayers <= 4 && range < 40)
            {
                range = 40;
            }

            GlobalServices::instance().logger()->verbose(
                "Overagression detected from the raiser, setting calling range to " + std::to_string(range));
        }
    }
    return range;
}
float PreflopRangeEstimator::finalizeRange(const float currentRange) const
{
    float range = std::ceil(currentRange);

    if (range < 1)
    {
        range = 1;
    }

    if (range > 100)
    {
        range = 100;
    }

    GlobalServices::instance().logger()->verbose("Estimated range is " + std::to_string(range) + " % ");
    return range;
}
std::string PreflopRangeEstimator::finalizeEstimatedRange(const CurrentHandContext& ctx,
                                                          std::vector<std::string>& ranges,
                                                          std::vector<float>& rangesValues, float range, int nbPlayers,
                                                          bool isTopRange) const
{
    if (!isTopRange)
    {
        GlobalServices::instance().logger()->verbose(" [ not a top range ] ");
        return RangeEstimator::getFilledRange(ranges, rangesValues, range, nbPlayers);
    }
    else
    {
        return RangeEstimator::getStringRange(nbPlayers, range);
    }
}

float PreflopRangeEstimator::handleLimpRange(const CurrentHandContext& ctx, const float currentRange) const
{
    float range = currentRange;

    if (myHand->getRunningPlayersList()->size() > 3)
    {
        range = currentRange - ctx.myStatistics.getPreflopStatistics().getPreflopRaise();
        GlobalServices::instance().logger()->verbose("Limp range adjusted for deception: " + std::to_string(range));
    }
    else
    {
        range = currentRange;
    }

    if (range < 5)
    {
        range = 5;
    }

    GlobalServices::instance().logger()->verbose("Limp range : " + std::to_string(range));
    return range;
}
float PreflopRangeEstimator::handleSingleRaiseRange(const CurrentHandContext& ctx, const float currentRange) const
{

    float range = currentRange - ctx.myStatistics.getPreflopStatistics().getPreflop3Bet();

    if (range < 1)
    {
        range = 1;
    }

    GlobalServices::instance().logger()->verbose("Single bet call range: " + std::to_string(range));

    if (ctx.myStatistics.getPreflopStatistics().getVoluntaryPutMoneyInPot() -
            ctx.myStatistics.getPreflopStatistics().getPreflopRaise() >
        15)
    {
        // Loose player adjustment
        range = range / 2;
        GlobalServices::instance().logger()->verbose("Loose player adjusted range: " + std::to_string(range));
    }

    return range;
}
float PreflopRangeEstimator::handleThreeBetRange(const CurrentHandContext& ctx, int opponentRaises,
                                                 const float currentRange) const
{

    float range = currentRange;

    if (ctx.myStatistics.getPreflopStatistics().m_hands < MIN_HANDS_STATISTICS_ACCURATE)
    {
        // Not enough hands, assume the opponent is an average tight player
        range = currentRange / 3;
        GlobalServices::instance().logger()->verbose("3-bet call range (default): " + std::to_string(range));
    }
    else
    {
        if (opponentRaises == 1)
        {
            // Player is being 3-betted
            range = ctx.myStatistics.getPreflopStatistics().getPreflopRaise() *
                    ctx.myStatistics.getPreflopStatistics().getPreflopCall3BetsFrequency() / 100;

            PreflopStatistics lastRaiserStats =
                ctx.preflopLastRaiser->getStatistics(ctx.nbPlayers).getPreflopStatistics();

            if (range < lastRaiserStats.getPreflop3Bet() * 0.8)
            {
                range = lastRaiserStats.getPreflop3Bet() * 0.8;
            }

            GlobalServices::instance().logger()->verbose("3-bet call range: " + std::to_string(range));
        }
        else
        {
            // Cold-calling a 3-bet
            range = ctx.myStatistics.getPreflopStatistics().getVoluntaryPutMoneyInPot() / 3;
            GlobalServices::instance().logger()->verbose("3-bet cold-call range: " + std::to_string(range));
        }
    }
    return range;
}
float PreflopRangeEstimator::handleFourBetOrMoreRange(const CurrentHandContext& ctx, int opponentRaises,
                                                      const float currentRange) const
{
    float range = currentRange;

    if (ctx.myStatistics.getPreflopStatistics().m_hands < MIN_HANDS_STATISTICS_ACCURATE)
    {
        // Not enough hands, assume the opponent is an average tight player
        range = currentRange / 5;
        GlobalServices::instance().logger()->verbose("4-bet call range (default): " + std::to_string(range));
    }
    else
    {
        if (opponentRaises > 0)
        {
            // Facing a 4-bet after having bet
            range = ctx.myStatistics.getPreflopStatistics().getPreflop3Bet() *
                    ctx.myStatistics.getPreflopStatistics().getPreflopCall3BetsFrequency() / 100;
            GlobalServices::instance().logger()->verbose("4-bet call range: " + std::to_string(range));
        }
        else
        {
            // Cold-calling a 4-bet
            range = ctx.myStatistics.getPreflopStatistics().getVoluntaryPutMoneyInPot() / 6;
            GlobalServices::instance().logger()->verbose("4-bet cold-call range: " + std::to_string(range));
        }
    }
    return range;
}

} // namespace pkt::core::player