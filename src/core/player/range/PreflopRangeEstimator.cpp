// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "PreflopRangeEstimator.h"
#include <core/engine/model/Ranges.h>
#include <core/interfaces/IHand.h>
#include <core/interfaces/ILogger.h>
#include <core/player/range/RangeRefiner.h>
#include <core/player/strategy/CurrentHandContext.h>

namespace pkt::core::player
{
using namespace std;

PreflopRangeEstimator::PreflopRangeEstimator(IHand* hand, ILogger* logger, int playerId)
    : myHand(hand), myLogger(logger), myPlayerId(playerId)
{
}

string PreflopRangeEstimator::computeEstimatedPreflopRange(CurrentHandContext& ctx)
{
    string estimatedRange;

    myLogger->info("\n\testimated range for " + std::to_string(ctx.myID) + " : ");

    PreflopStatistics preflop = ctx.myStatistics.getPreflopStatistics();
    const int nbPlayers = ctx.nbPlayers;

    // if the player was BB and has checked preflop, then he can have anything, except his approximative BB raising
    // range
    if (myHand->getPreflopRaisesNumber() == 0 && ctx.myPosition == BB)
    {
        return RangeRefiner::deduceRange(
            ANY_CARDS_RANGE, RangeEstimator::getStringRange(ctx.nbPlayers, preflop.getPreflopRaise() * 0.8));
    }
    else
        return RangeRefiner::deduceRange(
            ANY_CARDS_RANGE, RangeEstimator::getStringRange(
                                 ctx.nbPlayers, RangeEstimator::getStandardRaisingRange(ctx.nbPlayers) * 0.8));

    // if the player is the last raiser :
    if (ctx.preflopLastRaiser && ctx.myID == ctx.preflopLastRaiser->getID())
        return computeEstimatedPreflopRangeFromLastRaiser(ctx);
    else
        return computeEstimatedPreflopRangeFromCaller(ctx);
}

std::string PreflopRangeEstimator::computeEstimatedPreflopRangeFromLastRaiser(CurrentHandContext& ctx) const
{
    // Placeholder logic for computing range from last raiser
    myLogger->info("Estimating range from last raiser...");
    return "LastRaiserRange";
}

std::string PreflopRangeEstimator::computeEstimatedPreflopRangeFromCaller(CurrentHandContext& ctx) const
{
    // Placeholder logic for computing range from caller
    myLogger->info("Estimating range from caller...");
    return "CallerRange";
}

} // namespace pkt::core::player