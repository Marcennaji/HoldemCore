// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "PreflopRangeEstimator.h"
#include <cmath>
#include <core/engine/model/Ranges.h>
#include <core/player/range/RangeRefiner.h>
#include <core/player/strategy/CurrentHandContext.h>
#include <core/services/ServiceContainer.h>
#include <core/interfaces/ServiceAdapter.h>
#include <string>
#include <vector>

namespace pkt::core::player
{
using namespace std;

PreflopRangeEstimator::PreflopRangeEstimator(int playerId) : m_playerId(playerId)
{
    // Legacy constructor - create direct interface from default service container
    auto defaultServices = std::make_shared<pkt::core::AppServiceContainer>();
    m_logger = std::shared_ptr<pkt::core::Logger>(defaultServices, &defaultServices->logger());
}

PreflopRangeEstimator::PreflopRangeEstimator(int playerId,
                                             std::shared_ptr<pkt::core::ServiceContainer> serviceContainer)
    : m_playerId(playerId)
{
    // Legacy constructor - create direct interface from ServiceContainer
    m_logger = std::shared_ptr<pkt::core::Logger>(serviceContainer, &serviceContainer->logger());
}

// ISP-compliant constructor
PreflopRangeEstimator::PreflopRangeEstimator(int playerId, std::shared_ptr<pkt::core::Logger> logger)
    : m_playerId(playerId), m_logger(logger)
{
}

pkt::core::Logger& PreflopRangeEstimator::getLogger() const
{
    assert(m_logger && "PreflopRangeEstimator requires ISP-compliant logger interface");
    return *m_logger;
}

string PreflopRangeEstimator::computeEstimatedPreflopRange(const CurrentHandContext& ctx)
{
    string estimatedRange;

    getLogger().verbose("\n\testimated range for " + std::to_string(ctx.personalContext.id) + " : ");

    PreflopStatistics preflop = ctx.personalContext.statistics.preflopStatistics;
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;

    getLogger().verbose(
        "  " + std::to_string(preflop.getVoluntaryPutMoneyInPot()) + "/" + std::to_string(preflop.getPreflopRaise()) +
        ", 3B: " + std::to_string(preflop.getPreflop3Bet()) + ", 4B: " + std::to_string(preflop.getPreflop4Bet()) +
        ", C3B: " + std::to_string(preflop.getPreflopCallthreeBetsFrequency()) +
        ", pot odd: " + std::to_string(ctx.commonContext.bettingContext.potOdd) + " " + "\n\t\t");

    // if the player was BB and has checked preflop, then he can have anything, except his approximative BB raising
    // range
    if (ctx.commonContext.bettingContext.preflopRaisesNumber == 0 &&
        ctx.personalContext.position == PlayerPosition::BigBlind)
    {
        const float raiseFactor = (preflop.hands >= MIN_HANDS_STATISTICS_ACCURATE)
                                      ? preflop.getPreflopRaise() * 0.8
                                      : RangeEstimator::getStandardRaisingRange(nbPlayers) * 0.8;

        const string excludeRange = RangeEstimator::getStringRange(nbPlayers, raiseFactor);
        getLogger().verbose("any cards except " + excludeRange + '\n');
        estimatedRange = RangeRefiner::deduceRange(ANY_CARDS_RANGE, excludeRange);
    }
    else
    {
        // if the player is the last raiser :
        if (ctx.commonContext.playersContext.preflopLastRaiser &&
            m_playerId == ctx.commonContext.playersContext.preflopLastRaiser->getId())
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

std::string PreflopRangeEstimator::computeEstimatedPreflopRangeFromLastRaiser(const CurrentHandContext& ctx) const
{
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    assert(ctx.commonContext.playersContext.preflopLastRaiser != nullptr);

    float range = 0;

    getLogger().verbose(" [ player is last raiser ] ");
    PreflopStatistics preflopOpponent = ctx.commonContext.playersContext.preflopLastRaiser->getStatisticsUpdater()
                                            ->getStatistics(nbPlayers)
                                            .preflopStatistics;

    if (ctx.personalContext.statistics.preflopStatistics.hands >= MIN_HANDS_STATISTICS_ACCURATE)
    {
        if (ctx.commonContext.bettingContext.preflopRaisesNumber == 1)
        {
            range = preflopOpponent.getPreflopRaise();
        }
        else
        {
            // Assume the player has adapted their raising range to the previous raiser
            if (preflopOpponent.hands >= MIN_HANDS_STATISTICS_ACCURATE)
            {
                if (ctx.commonContext.bettingContext.preflopRaisesNumber == 2)
                {
                    range = preflopOpponent.getPreflopRaise() * 0.7;
                }
                else if (ctx.commonContext.bettingContext.preflopRaisesNumber == 3)
                {
                    range = preflopOpponent.getPreflop3Bet() * 0.7;
                }
                else if (ctx.commonContext.bettingContext.preflopRaisesNumber > 3)
                {
                    range =
                        preflopOpponent.getPreflop4Bet() / (ctx.commonContext.bettingContext.preflopRaisesNumber / 2);
                }
            }
            else
            {
                if (ctx.commonContext.bettingContext.preflopRaisesNumber == 2)
                {
                    range = preflopOpponent.getPreflopRaise();
                }
                else if (ctx.commonContext.bettingContext.preflopRaisesNumber == 3)
                {
                    range = preflopOpponent.getPreflop3Bet();
                }
                else if (ctx.commonContext.bettingContext.preflopRaisesNumber > 3)
                {
                    range =
                        preflopOpponent.getPreflop4Bet() / (ctx.commonContext.bettingContext.preflopRaisesNumber / 2);
                }
            }
        }
    }
    else
    {
        range = RangeEstimator::getStandardRaisingRange(nbPlayers);

        getLogger().verbose(", but not enough hands -> getting the standard range : " + std::to_string(range));

        if (ctx.commonContext.bettingContext.preflopRaisesNumber == 2)
        {
            range = range * 0.3;
        }
        else if (ctx.commonContext.bettingContext.preflopRaisesNumber == 3)
        {
            range = range * 0.2;
        }
        else if (ctx.commonContext.bettingContext.preflopRaisesNumber > 3)
        {
            range = range * 0.1;
        }
    }

    getLogger().verbose("range is " + std::to_string(range));

    if (nbPlayers > 3)
    {
        // Adjust range based on position
        if (ctx.commonContext.playersContext.preflopLastRaiser->getPosition() == UnderTheGun ||
            ctx.commonContext.playersContext.preflopLastRaiser->getPosition() == UnderTheGunPlusOne ||
            ctx.commonContext.playersContext.preflopLastRaiser->getPosition() == UnderTheGunPlusTwo)
        {
            range = range * 0.9;
        }
        else if (ctx.commonContext.playersContext.preflopLastRaiser->getPosition() == Button ||
                 ctx.commonContext.playersContext.preflopLastRaiser->getPosition() == Cutoff)
        {
            range = range * 1.5;
        }

        getLogger().verbose(", position adjusted range is " + std::to_string(range));
    }

    // Adjust range for loose/aggressive mode
    if (ctx.commonContext.playersContext.preflopLastRaiser &&
        ctx.commonContext.playersContext.preflopLastRaiser->isInVeryLooseMode(nbPlayers))
    {
        if (range < 40)
        {
            range = 40;
            getLogger().verbose("\t\toveragression detected, setting range to " + std::to_string(range));
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

    getLogger().verbose("\n\t\testimated range is " + std::to_string(range) + " % ");

    return RangeEstimator::getStringRange(nbPlayers, range);
}

std::string PreflopRangeEstimator::computeEstimatedPreflopRangeFromCaller(const CurrentHandContext& ctx) const
{
    // The player is not the last raiser but has called a raise or limped in preflop

    bool isTopRange = true;
    std::vector<std::string> ranges;
    std::vector<float> rangesValues;
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
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

    getLogger().verbose("Estimated range for player " + std::to_string(ctx.personalContext.id) + " is {" +
                                 rangeString + "}");
    return rangeString;
}
void PreflopRangeEstimator::analyzePlayerActions(const CurrentHandContext& ctx, int& raises, int& calls) const
{
    for (const auto& action : ctx.personalContext.actions.currentHandActions.getActions(GameState::Preflop))
    {
        if (action.type == ActionType::Raise || action.type == ActionType::Allin)
        {
            raises++;
        }
        else if (action.type == ActionType::Call)
        {
            calls++;
        }
    }
}
float PreflopRangeEstimator::calculateStartingRange(const CurrentHandContext& ctx, int nbPlayers) const
{
    float estimatedStartingRange = ctx.personalContext.statistics.preflopStatistics.getVoluntaryPutMoneyInPot();

    if (ctx.personalContext.statistics.preflopStatistics.hands < MIN_HANDS_STATISTICS_ACCURATE)
    {
        estimatedStartingRange = RangeEstimator::getStandardCallingRange(nbPlayers);
        getLogger().verbose(" [ not enough hands, getting the standard calling range ] ");
    }

    getLogger().verbose(" estimated starting range : " + std::to_string(estimatedStartingRange));
    return estimatedStartingRange;
}
float PreflopRangeEstimator::adjustRangeForPosition(const CurrentHandContext& ctx, int nbPlayers,
                                                    const float currentRange) const
{
    float range = currentRange;

    if (nbPlayers > 3)
    {
        if (ctx.personalContext.position == UnderTheGun || ctx.personalContext.position == UnderTheGunPlusOne ||
            ctx.personalContext.position == UnderTheGunPlusTwo)
        {
            range *= 0.9;
        }
        else if (ctx.personalContext.position == Button || ctx.personalContext.position == Cutoff)
        {
            range *= 1.4;
        }
    }

    if (range > 100)
    {
        range = 100;
    }

    getLogger().verbose("Position adjusted starting range : " + std::to_string(range));
    return range;
}
float PreflopRangeEstimator::adjustRangeForPotOdds(const CurrentHandContext& ctx, const float currentRange) const
{
    const int potOdd = ctx.commonContext.bettingContext.potOdd;
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
    else if (potOdd <= 20 && ctx.commonContext.bettingContext.preflopRaisesNumber < 2)
    {
        range = 40;
    }

    getLogger().verbose("Pot odd adjusted starting range : " + std::to_string(range));
    return range;
}
float PreflopRangeEstimator::adjustRangeForPreflopRaises(const CurrentHandContext& ctx, int opponentRaises,
                                                         const float currentRange) const
{
    float range = currentRange;
    if (ctx.commonContext.bettingContext.preflopRaisesNumber == 0)
    {
        range = handleLimpRange(ctx, range);
    }
    else if (ctx.commonContext.bettingContext.preflopRaisesNumber == 1)
    {
        range = handleSingleRaiseRange(ctx, range);
    }
    else if (ctx.commonContext.bettingContext.preflopRaisesNumber == 2)
    {
        range = handleThreeBetRange(ctx, opponentRaises, range);
    }
    else if (ctx.commonContext.bettingContext.preflopRaisesNumber > 2)
    {
        range = handleFourBetOrMoreRange(ctx, opponentRaises, range);
    }
    return range;
}
float PreflopRangeEstimator::handleLooseAggressiveOpponents(const CurrentHandContext& ctx, int opponentRaises,
                                                            const float currentRange) const
{
    if (ctx.commonContext.playersContext.preflopLastRaiser == nullptr)
        return currentRange;

    const int lastRaiserId = ctx.commonContext.playersContext.preflopLastRaiser->getId();
    float range = currentRange;

    if (opponentRaises > 0 && lastRaiserId != -1 && ctx.commonContext.bettingContext.preflopRaisesNumber == 1)
    {
        if (ctx.commonContext.playersContext.preflopLastRaiser->isInVeryLooseMode(
                ctx.commonContext.playersContext.nbPlayers))
        {
            if (ctx.commonContext.playersContext.nbPlayers > 6 && range < 20)
            {
                range = 20;
            }
            else if (ctx.commonContext.playersContext.nbPlayers > 4 && range < 30)
            {
                range = 30;
            }
            else if (ctx.commonContext.playersContext.nbPlayers <= 4 && range < 40)
            {
                range = 40;
            }

            getLogger().verbose("Overagression detected from the raiser, setting calling range to " +
                                         std::to_string(range));
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

    getLogger().verbose("Estimated range is " + std::to_string(range) + " % ");
    return range;
}
std::string PreflopRangeEstimator::finalizeEstimatedRange(const CurrentHandContext& ctx,
                                                          std::vector<std::string>& ranges,
                                                          std::vector<float>& rangesValues, float range, int nbPlayers,
                                                          bool isTopRange) const
{
    if (!isTopRange)
    {
        getLogger().verbose(" [ not a top range ] ");
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

    if (ctx.commonContext.playersContext.actingPlayersList->size() > 3)
    {
        range = currentRange - ctx.personalContext.statistics.preflopStatistics.getPreflopRaise();
        getLogger().verbose("Limp range adjusted for deception: " + std::to_string(range));
    }
    else
    {
        range = currentRange;
    }

    if (range < 5)
    {
        range = 5;
    }

    getLogger().verbose("Limp range : " + std::to_string(range));
    return range;
}
float PreflopRangeEstimator::handleSingleRaiseRange(const CurrentHandContext& ctx, const float currentRange) const
{

    float range = currentRange - ctx.personalContext.statistics.preflopStatistics.getPreflop3Bet();

    if (range < 1)
    {
        range = 1;
    }

    getLogger().verbose("Single bet call range: " + std::to_string(range));

    if (ctx.personalContext.statistics.preflopStatistics.getVoluntaryPutMoneyInPot() -
            ctx.personalContext.statistics.preflopStatistics.getPreflopRaise() >
        15)
    {
        // Loose player adjustment
        range = range / 2;
        getLogger().verbose("Loose player adjusted range: " + std::to_string(range));
    }

    return range;
}
float PreflopRangeEstimator::handleThreeBetRange(const CurrentHandContext& ctx, int opponentRaises,
                                                 const float currentRange) const
{

    float range = currentRange;

    if (ctx.personalContext.statistics.preflopStatistics.hands < MIN_HANDS_STATISTICS_ACCURATE)
    {
        // Not enough hands, assume the opponent is an average tight player
        range = currentRange / 3;
        getLogger().verbose("3-bet call range (default): " + std::to_string(range));
    }
    else
    {
        if (opponentRaises == 1)
        {
            // Player is being 3-betted
            range = ctx.personalContext.statistics.preflopStatistics.getPreflopRaise() *
                    ctx.personalContext.statistics.preflopStatistics.getPreflopCallthreeBetsFrequency() / 100;

            PreflopStatistics lastRaiserStats =
                ctx.commonContext.playersContext.preflopLastRaiser->getStatisticsUpdater()
                    ->getStatistics(ctx.commonContext.playersContext.nbPlayers)
                    .preflopStatistics;

            if (range < lastRaiserStats.getPreflop3Bet() * 0.8)
            {
                range = lastRaiserStats.getPreflop3Bet() * 0.8;
            }

            getLogger().verbose("3-bet call range: " + std::to_string(range));
        }
        else
        {
            // Cold-calling a 3-bet
            range = ctx.personalContext.statistics.preflopStatistics.getVoluntaryPutMoneyInPot() / 3;
            getLogger().verbose("3-bet cold-call range: " + std::to_string(range));
        }
    }
    return range;
}
float PreflopRangeEstimator::handleFourBetOrMoreRange(const CurrentHandContext& ctx, int opponentRaises,
                                                      const float currentRange) const
{
    float range = currentRange;

    if (ctx.personalContext.statistics.preflopStatistics.hands < MIN_HANDS_STATISTICS_ACCURATE)
    {
        // Not enough hands, assume the opponent is an average tight player
        range = currentRange / 5;
        getLogger().verbose("4-bet call range (default): " + std::to_string(range));
    }
    else
    {
        if (opponentRaises > 0)
        {
            // Facing a 4-bet after having bet
            range = ctx.personalContext.statistics.preflopStatistics.getPreflop3Bet() *
                    ctx.personalContext.statistics.preflopStatistics.getPreflopCallthreeBetsFrequency() / 100;
            getLogger().verbose("4-bet call range: " + std::to_string(range));
        }
        else
        {
            // Cold-calling a 4-bet
            range = ctx.personalContext.statistics.preflopStatistics.getVoluntaryPutMoneyInPot() / 6;
            getLogger().verbose("4-bet cold-call range: " + std::to_string(range));
        }
    }
    return range;
}

} // namespace pkt::core::player