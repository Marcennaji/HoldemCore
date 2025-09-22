// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#include <core/player/strategy/ManiacBotStrategy.h>

#include <core/engine/hand/HandEvaluator.h>

#include <core/engine/model/EngineError.h>
#include <core/engine/model/Ranges.h>
#include <core/player/Helpers.h>
#include <core/player/strategy/CurrentHandContext.h>
#include "Exception.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

namespace pkt::core::player
{

using namespace std;

ManiacBotStrategy::ManiacBotStrategy()
{
    // initialize utg starting range, in a full table
    int utgFullTableRange = 0;
    ensureServicesInitialized();
    myServices->randomizer().getRand(30, 35, 1, &utgFullTableRange);
    initializeRanges(50, utgFullTableRange);
}

ManiacBotStrategy::ManiacBotStrategy(std::shared_ptr<pkt::core::ServiceContainer> serviceContainer)
    : BotStrategyBase(serviceContainer)
{
    // initialize utg starting range, in a full table
    int utgFullTableRange = 0;
    myServices->randomizer().getRand(30, 35, 1, &utgFullTableRange);
    initializeRanges(50, utgFullTableRange);
}

ManiacBotStrategy::~ManiacBotStrategy() = default;

bool ManiacBotStrategy::preflopShouldCall(const CurrentHandContext& ctx)
{
    float callingRange = getPreflopRangeCalculator()->calculatePreflopCallingRange(ctx);
    if (callingRange == -1)
    {
        return false; // never call : raise or fold
    }

    string stringCallingRange;

    const char** rangesString;

    if (ctx.commonContext.playersContext.nbPlayers == 2)
    {
        rangesString = TOP_RANGE_2_PLAYERS;
    }
    else if (ctx.commonContext.playersContext.nbPlayers == 3)
    {
        rangesString = TOP_RANGE_3_PLAYERS;
    }
    else if (ctx.commonContext.playersContext.nbPlayers == 4)
    {
        rangesString = TOP_RANGE_4_PLAYERS;
    }
    else
    {
        rangesString = TOP_RANGE_MORE_4_PLAYERS;
    }

    stringCallingRange = rangesString[(int) callingRange];

    if (ctx.commonContext.bettingContext.preflopRaisesNumber < 3)
    {
        myServices->logger().verbose("\t\tManiac adding high pairs to the initial calling range.");
        stringCallingRange += HIGH_PAIRS;
    }

    std::shared_ptr<Player> lastRaiser = ctx.commonContext.playersContext.preflopLastRaiser;

    if (ctx.commonContext.bettingContext.preflopRaisesNumber < 2 &&
        ctx.personalContext.cash >= ctx.commonContext.bettingContext.pot * 10 && lastRaiser != nullptr &&
        lastRaiser->getCash() >= ctx.commonContext.bettingContext.pot * 10 &&
        !ctx.commonContext.bettingContext.isPreflopBigBet)
    {

        myServices->logger().verbose(
            "\t\tManiac adding high suited connectors, high suited aces and pairs to the initial calling range.");
        stringCallingRange += HIGH_SUITED_CONNECTORS;
        stringCallingRange += HIGH_SUITED_ACES;
        stringCallingRange += PAIRS;

        if (ctx.commonContext.playersContext.actingPlayersList->size() > 2 &&
            ctx.commonContext.bettingContext.preflopRaisesNumber + ctx.commonContext.bettingContext.preflopCallsNumber >
                1 &&
            ctx.personalContext.position >= Middle)
        {
            stringCallingRange += CONNECTORS;
            stringCallingRange += SUITED_ONE_GAPED;
            stringCallingRange += SUITED_TWO_GAPED;
            myServices->logger().verbose(
                "\t\tManiac adding suited connectors, suited one-gaped and suited two-gaped to the initial "
                "calling range.");
        }
    }

    // defend against 3bet bluffs :
    if (ctx.commonContext.bettingContext.preflopRaisesNumber == 2 &&
        ctx.personalContext.actions.currentHandActions.getActions(GameState::Preflop).size() > 0 &&
        ctx.personalContext.actions.currentHandActions.getActions(GameState::Preflop).back().type ==
            ActionType::Raise &&
        ctx.personalContext.cash >= ctx.commonContext.bettingContext.pot * 10 && lastRaiser != nullptr &&
        lastRaiser->getCash() >= ctx.commonContext.bettingContext.pot * 10 &&
        !ctx.commonContext.bettingContext.isPreflopBigBet)
    {

        int rand = 0;
        myServices->randomizer().getRand(1, 3, 1, &rand);
        if (rand == 1)
        {

            stringCallingRange += HIGH_SUITED_CONNECTORS;
            stringCallingRange += HIGH_SUITED_ACES;
            stringCallingRange += PAIRS;

            myServices->logger().verbose(
                "\t\tManiac defending against 3-bet : adding high suited connectors, high suited aces and pairs to "
                "the initial calling range.");
        }
    }
    myServices->logger().verbose("\t\tManiac final calling range : " + stringCallingRange);

    return isCardsInRange(ctx.personalContext.holeCards, stringCallingRange);
}

int ManiacBotStrategy::preflopShouldRaise(const CurrentHandContext& ctx)
{
    float raisingRange = getPreflopRangeCalculator()->calculatePreflopRaisingRange(ctx);

    if (raisingRange == -1)
    {
        return 0; // never raise : call or fold
    }

    if (ctx.commonContext.bettingContext.preflopRaisesNumber > 3)
    {
        return 0; // never 6-bet : call or fold
    }

    string stringRaisingRange;

    const char** rangesString;

    if (ctx.commonContext.playersContext.nbPlayers == 2)
    {
        rangesString = TOP_RANGE_2_PLAYERS;
    }
    else if (ctx.commonContext.playersContext.nbPlayers == 3)
    {
        rangesString = TOP_RANGE_3_PLAYERS;
    }
    else if (ctx.commonContext.playersContext.nbPlayers == 4)
    {
        rangesString = TOP_RANGE_4_PLAYERS;
    }
    else
    {
        rangesString = TOP_RANGE_MORE_4_PLAYERS;
    }

    stringRaisingRange = rangesString[(int) raisingRange];

    myServices->logger().verbose(stringRaisingRange);

    // determine when to 3-bet without a real hand
    bool speculativeHandedAdded = false;

    if (ctx.commonContext.bettingContext.preflopRaisesNumber == 1)
    {
        PreflopStatistics raiserStats = ctx.commonContext.playersContext.preflopLastRaiser->getStatisticsUpdater()
                                            ->getStatistics(ctx.commonContext.playersContext.nbPlayers)
                                            .preflopStatistics;

        if (!isCardsInRange(ctx.personalContext.holeCards, stringRaisingRange) && ctx.personalContext.m > 20 &&
            ctx.personalContext.cash > ctx.commonContext.bettingContext.highestBetAmount * 20 &&
            ctx.personalContext.position > UnderTheGunPlusTwo && raiserStats.hands > MIN_HANDS_STATISTICS_ACCURATE &&
            ctx.personalContext.position > ctx.commonContext.playersContext.preflopLastRaiser->getPosition() &&
            ctx.commonContext.playersContext.preflopLastRaiser->getCash() >
                ctx.commonContext.bettingContext.highestBetAmount * 10 &&
            !ctx.commonContext.bettingContext.isPreflopBigBet &&
            ctx.commonContext.bettingContext.preflopCallsNumber < 2)
        {
            if (isPossibleToBluff(ctx) && ctx.personalContext.position > Late &&
                ctx.commonContext.bettingContext.preflopRaisesNumber == 1 &&
                !isCardsInRange(ctx.personalContext.holeCards, ACES + BROADWAYS) &&
                raiserStats.getPreflopCallthreeBetsFrequency() < 30)
            {

                speculativeHandedAdded = true;
                myServices->logger().verbose("\t\tManiac trying to steal this bet");
            }
            else
            {
                if (isCardsInRange(ctx.personalContext.holeCards,
                                   LOW_PAIRS + SUITED_CONNECTORS + SUITED_ONE_GAPED + SUITED_TWO_GAPED))
                {

                    speculativeHandedAdded = true;
                    myServices->logger().verbose(
                        "\t\tManiac adding this speculative hand to our initial raising range");
                }
                else
                {
                    if (!isCardsInRange(ctx.personalContext.holeCards, PAIRS + ACES + BROADWAYS) &&
                        raiserStats.getPreflopCallthreeBetsFrequency() < 30)
                    {

                        int rand = 0;
                        myServices->randomizer().getRand(1, 3, 1, &rand);
                        if (rand == 1)
                        {
                            speculativeHandedAdded = true;
                            myServices->logger().verbose(
                                "\t\tManiac adding this junk hand to our initial raising range");
                        }
                    }
                }
            }
        }
    }
    // determine when to 4-bet without a real hand
    if (!speculativeHandedAdded && ctx.commonContext.bettingContext.preflopRaisesNumber == 2)
    {
        PreflopStatistics raiserStats = ctx.commonContext.playersContext.preflopLastRaiser->getStatisticsUpdater()
                                            ->getStatistics(ctx.commonContext.playersContext.nbPlayers)
                                            .preflopStatistics;

        if (!isCardsInRange(ctx.personalContext.holeCards, stringRaisingRange) &&
            !isCardsInRange(ctx.personalContext.holeCards, OFFSUITED_BROADWAYS) && ctx.personalContext.m > 20 &&
            ctx.personalContext.cash > ctx.commonContext.bettingContext.highestBetAmount * 60 &&
            ctx.personalContext.position > MiddlePlusOne && raiserStats.hands > MIN_HANDS_STATISTICS_ACCURATE &&
            ctx.personalContext.position > ctx.commonContext.playersContext.preflopLastRaiser->getPosition() &&
            ctx.commonContext.playersContext.preflopLastRaiser->getCash() >
                ctx.commonContext.bettingContext.highestBetAmount * 20 &&
            !ctx.commonContext.bettingContext.isPreflopBigBet &&
            ctx.commonContext.bettingContext.preflopCallsNumber < 2)
        {

            if (isPossibleToBluff(ctx) && ctx.personalContext.position > Late && raiserStats.getPreflop3Bet() > 8)
            {
                int rand = 0;
                myServices->randomizer().getRand(1, 5, 1, &rand);
                if (rand == 1)
                {
                    speculativeHandedAdded = true;
                    myServices->logger().verbose(
                        "\t\tManiac adding this speculative hand to our initial raising range");
                }
            }
        }
    }
    if (!speculativeHandedAdded && !isCardsInRange(ctx.personalContext.holeCards, stringRaisingRange))
    {
        return 0;
    }

    // sometimes, just call a single raise instead of raising, even with a strong hand
    // nb. raising range 100 means that I want to steal a bet or BB
    if (!speculativeHandedAdded && ctx.commonContext.bettingContext.preflopCallsNumber == 0 &&
        ctx.commonContext.bettingContext.preflopRaisesNumber == 1 && raisingRange < 100 &&
        !(isCardsInRange(ctx.personalContext.holeCards, LOW_PAIRS + MEDIUM_PAIRS) &&
          ctx.commonContext.playersContext.nbPlayers < 4) &&
        !(isCardsInRange(ctx.personalContext.holeCards, HIGH_PAIRS) &&
          ctx.commonContext.bettingContext.preflopCallsNumber > 0) &&
        isCardsInRange(ctx.personalContext.holeCards,
                       RangeEstimator::getStringRange(ctx.commonContext.playersContext.nbPlayers, 4)))
    {

        int rand = 0;
        myServices->randomizer().getRand(1, 10, 1, &rand);
        if (rand == 1)
        {
            myServices->logger().verbose("\t\twon't raise, to hide the hand strength");
            myShouldCall = true;
            return 0;
        }
    }

    return computePreflopRaiseAmount(ctx);
}

int ManiacBotStrategy::flopShouldBet(const CurrentHandContext& ctx)
{

    if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber > 0)
    {
        return 0;
    }

    if (shouldPotControl(ctx))
    {
        return 0;
    }

    // donk bets :
    if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber > 0 &&
        ctx.commonContext.bettingContext.preflopRaisesNumber > 0 &&
        ctx.commonContext.playersContext.preflopLastRaiser->getId() != ctx.personalContext.id)
    {
        if (ctx.commonContext.playersContext.preflopLastRaiser->getPosition() > ctx.personalContext.position)
        {

            if (getDrawingProbability(ctx.personalContext.postFlopAnalysisFlags) > 25)
            {
                int rand = 0;
                myServices->randomizer().getRand(1, 2, 1, &rand);
                if (rand == 1)
                {
                    return ctx.commonContext.bettingContext.pot * 0.6;
                }
            }

            if ((ctx.personalContext.postFlopAnalysisFlags.isTwoPair ||
                 ctx.personalContext.postFlopAnalysisFlags.isTrips ||
                 ctx.personalContext.postFlopAnalysisFlags.isStraight) &&
                ctx.personalContext.postFlopAnalysisFlags.isFlushDrawPossible)
            {
                return ctx.commonContext.bettingContext.pot * 0.6;
            }

            // if the flop is dry, try to get the pot
            if (ctx.commonContext.playersContext.nbPlayers < 3 && isPossibleToBluff(ctx) &&
                getBoardCardsHigherThan(ctx.commonContext.stringBoard, "Jh") < 2 &&
                getBoardCardsHigherThan(ctx.commonContext.stringBoard, "Kh") == 0 &&
                !ctx.personalContext.postFlopAnalysisFlags.isFlushDrawPossible)
            {

                int rand = 0;
                myServices->randomizer().getRand(1, 2, 1, &rand);
                if (rand == 1)
                {
                    return ctx.commonContext.bettingContext.pot * 0.6;
                }
            }
        }
    }

    // don't bet if in position, and pretty good drawing probs
    if (getDrawingProbability(ctx.personalContext.postFlopAnalysisFlags) > 20 && ctx.personalContext.hasPosition)
    {
        return 0;
    }

    // if pretty good hand
    if (ctx.personalContext.myHandSimulation.winRanged > 0.5 || ctx.personalContext.myHandSimulation.win > 0.9)
    {

        // always bet if my hand will lose a lot of its value on next betting rounds
        if (ctx.personalContext.myHandSimulation.winRanged - ctx.personalContext.myHandSimulation.winSd > 0.1)
        {
            return ctx.commonContext.bettingContext.pot * 0.8;
        }

        // if no raise preflop, or if more than 1 opponent
        if (ctx.commonContext.bettingContext.preflopRaisesNumber == 0 ||
            ctx.commonContext.playersContext.actingPlayersList->size() > 2)
        {

            if (ctx.commonContext.playersContext.actingPlayersList->size() < 4)
            {
                return ctx.commonContext.bettingContext.pot * 0.8;
            }
            else
            {
                return ctx.commonContext.bettingContext.pot * 1.2;
            }
        }

        // if i have raised preflop, bet
        if (ctx.commonContext.bettingContext.preflopRaisesNumber > 0 &&
            ctx.commonContext.playersContext.preflopLastRaiser->getId() == ctx.personalContext.id)
        {
            return ctx.commonContext.bettingContext.pot * 0.8;
        }
    }
    else
    {

        ///////////  if bad flop for me

        // if there was a lot of action preflop, and i was not the last raiser : don't bet
        if (ctx.commonContext.bettingContext.preflopRaisesNumber > 2 &&
            ctx.commonContext.playersContext.preflopLastRaiser->getId() != ctx.personalContext.id)
        {
            return 0;
        }

        // if I was the last raiser preflop, I may bet with not much
        if (ctx.commonContext.bettingContext.preflopRaisesNumber > 0 &&
            ctx.commonContext.playersContext.preflopLastRaiser->getId() == ctx.personalContext.id &&
            ctx.commonContext.playersContext.actingPlayersList->size() < 4 &&
            ctx.personalContext.cash > ctx.commonContext.bettingContext.pot * 4 && isPossibleToBluff(ctx))
        {
            if (ctx.personalContext.myHandSimulation.winRanged > 0.15 && ctx.personalContext.myHandSimulation.win > 0.3)
            {
                return ctx.commonContext.bettingContext.pot * 0.8;
            }
        }
    }

    return 0;
}
bool ManiacBotStrategy::flopShouldCall(const CurrentHandContext& ctx)
{

    if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber == 0)
    {
        return false;
    }

    if (isDrawingProbOk(ctx.personalContext.postFlopAnalysisFlags, ctx.commonContext.bettingContext.potOdd))
    {
        return true;
    }

    if (ctx.personalContext.myHandSimulation.winRanged == 1 && ctx.personalContext.myHandSimulation.win > 0.5)
    {
        return true;
    }

    if (ctx.personalContext.myHandSimulation.winRanged * 100 < ctx.commonContext.bettingContext.potOdd * 0.8 &&
        ctx.personalContext.myHandSimulation.win < 0.9)
    {
        return false;
    }

    if (ctx.personalContext.myHandSimulation.winRanged < 0.25)
    {
        return false;
    }

    return true;
}

int ManiacBotStrategy::flopShouldRaise(const CurrentHandContext& ctx)
{

    const int nbRaises = ctx.commonContext.bettingContext.flopBetsOrRaisesNumber;

    if (nbRaises == 0)
    {
        return 0;
    }

    if (shouldPotControl(ctx))
    {
        return 0;
    }

    if (nbRaises < 2 && ctx.commonContext.playersContext.actingPlayersList->size() < 4 && isPossibleToBluff(ctx) &&
        ctx.personalContext.myHandSimulation.winRanged < 0.3 &&
        getBoardCardsHigherThan(ctx.commonContext.stringBoard, "Jh") < 2 &&
        getBoardCardsHigherThan(ctx.commonContext.stringBoard, "Kh") == 0)
    {

        int rand = 0;
        myServices->randomizer().getRand(1, 3, 1, &rand);
        if (rand == 2)
        {
            return ctx.commonContext.bettingContext.pot * 2;
        }
    }

    if (nbRaises == 2 && ctx.personalContext.myHandSimulation.win < 0.95)
    {
        return 0;
    }

    if (nbRaises == 3 && ctx.personalContext.myHandSimulation.win < 0.98)
    {
        return 0;
    }

    if (nbRaises > 3 && ctx.personalContext.myHandSimulation.win != 1)
    {
        return 0;
    }

    if ((isDrawingProbOk(ctx.personalContext.postFlopAnalysisFlags, ctx.commonContext.bettingContext.potOdd) ||
         ctx.personalContext.hasPosition) &&
        ctx.commonContext.playersContext.actingPlayersList->size() == 2 &&
        !(ctx.personalContext.myHandSimulation.winRanged * 100 < ctx.commonContext.bettingContext.potOdd) &&
        isPossibleToBluff(ctx) && nbRaises < 2)
    {

        int rand = 0;
        myServices->randomizer().getRand(1, 2, 1, &rand);
        if (rand == 2)
        {
            return ctx.commonContext.bettingContext.pot;
        }
    }

    if (ctx.personalContext.myHandSimulation.winRanged > 0.9 && ctx.personalContext.myHandSimulation.win > 0.5 &&
        nbRaises < 3)
    {
        return ctx.commonContext.bettingContext.pot;
    }
    if (ctx.personalContext.myHandSimulation.winRanged > 0.8 && ctx.personalContext.myHandSimulation.win > 0.5 &&
        nbRaises < 2)
    {
        return ctx.commonContext.bettingContext.pot;
    }

    return 0;
}

int ManiacBotStrategy::turnShouldBet(const CurrentHandContext& ctx)
{

    const int pot = ctx.commonContext.bettingContext.pot + ctx.commonContext.bettingContext.sets;
    const int nbRaises = ctx.commonContext.bettingContext.turnBetsOrRaisesNumber;

    if (nbRaises > 0)
    {
        return 0;
    }

    if (shouldPotControl(ctx))
    {
        return 0;
    }

    if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber > 1 && !ctx.personalContext.actions.flopIsAggressor &&
        ctx.personalContext.myHandSimulation.winRanged < 0.8)
    {
        return 0;
    }

    if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber == 0 && ctx.personalContext.hasPosition)
    {
        return pot * 0.8;
    }

    if (ctx.personalContext.myHandSimulation.winRanged < 0.3 && ctx.personalContext.myHandSimulation.win < 0.9 &&
        !ctx.personalContext.hasPosition)
    {
        return 0;
    }

    if (ctx.personalContext.myHandSimulation.winRanged > 0.4 && ctx.personalContext.myHandSimulation.win > 0.5 &&
        ctx.personalContext.hasPosition)
    {
        return pot * 0.8;
    }

    if (getDrawingProbability(ctx.personalContext.postFlopAnalysisFlags) > 15 && !ctx.personalContext.hasPosition)
    {
        return pot * 0.8;
    }
    else
    {
        // no draw, not a good hand, but last to speak and nobody has bet
        if (ctx.personalContext.hasPosition && isPossibleToBluff(ctx))
        {
            return pot * 0.8;
        }
    }

    return 0;
}

bool ManiacBotStrategy::turnShouldCall(const CurrentHandContext& ctx)
{
    if (ctx.commonContext.bettingContext.turnBetsOrRaisesNumber == 0)
    {
        return false;
    }

    if (isDrawingProbOk(ctx.personalContext.postFlopAnalysisFlags, ctx.commonContext.bettingContext.potOdd))
    {
        return true;
    }

    TurnStatistics raiserStats = ctx.commonContext.playersContext.turnLastRaiser->getStatisticsUpdater()
                                     ->getStatistics(ctx.commonContext.playersContext.nbPlayers)
                                     .turnStatistics;

    // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more
    // accurate
    if (raiserStats.hands < MIN_HANDS_STATISTICS_ACCURATE && ctx.commonContext.playersContext.nbPlayers < 10 &&
        ctx.commonContext.playersContext.turnLastRaiser->getStatisticsUpdater()
                ->getStatistics(ctx.commonContext.playersContext.nbPlayers + 1)
                .turnStatistics.hands > MIN_HANDS_STATISTICS_ACCURATE)
    {

        raiserStats = ctx.commonContext.playersContext.turnLastRaiser->getStatisticsUpdater()
                          ->getStatistics(ctx.commonContext.playersContext.nbPlayers + 1)
                          .turnStatistics;
    }

    if (ctx.personalContext.myHandSimulation.winRanged * 100 < ctx.commonContext.bettingContext.potOdd &&
        ctx.personalContext.myHandSimulation.winRanged < 0.94)
    {
        return false;
    }

    if (ctx.commonContext.bettingContext.turnBetsOrRaisesNumber == 2 &&
        ctx.personalContext.myHandSimulation.winRanged < 0.8 && ctx.personalContext.myHandSimulation.win < 0.9)
    {
        if (raiserStats.hands <= MIN_HANDS_STATISTICS_ACCURATE)
        {
            return false;
        }
        if (raiserStats.getAgressionFrequency() < 20)
        {
            return false;
        }
    }
    if (ctx.commonContext.bettingContext.turnBetsOrRaisesNumber > 2 &&
        ctx.personalContext.myHandSimulation.winRanged < 0.8 && ctx.personalContext.myHandSimulation.win < 0.9)
    {
        if (raiserStats.hands <= MIN_HANDS_STATISTICS_ACCURATE)
        {
            return false;
        }
        if (raiserStats.getAgressionFrequency() < 20)
        {
            return false;
        }
    }

    if (ctx.personalContext.myHandSimulation.winRanged < 0.5 &&
        (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber > 0 || raiserStats.getAgressionFrequency() < 30))
    {
        return false;
    }

    if (!ctx.personalContext.actions.preflopIsAggressor && !ctx.personalContext.actions.flopIsAggressor &&
        ctx.personalContext.myHandSimulation.winRanged < 0.7 && raiserStats.getAgressionFrequency() < 30 &&
        !ctx.personalContext.hasPosition)
    {
        return false;
    }

    if (ctx.personalContext.myHandSimulation.winRanged < 0.25 && ctx.personalContext.myHandSimulation.win < 0.9)
    {
        return false;
    }

    return true;
}

int ManiacBotStrategy::turnShouldRaise(const CurrentHandContext& ctx)
{
    if (ctx.commonContext.bettingContext.turnBetsOrRaisesNumber == 0)
    {
        return 0;
    }

    if (shouldPotControl(ctx))
    {
        return 0;
    }

    if (ctx.commonContext.bettingContext.turnBetsOrRaisesNumber == 1 && ctx.personalContext.myHandSimulation.win < 0.9)
    {
        return 0;
    }

    if (ctx.commonContext.bettingContext.turnBetsOrRaisesNumber == 2 && ctx.personalContext.myHandSimulation.win < 0.94)
    {
        return 0;
    }

    if (ctx.commonContext.bettingContext.turnBetsOrRaisesNumber > 2 && ctx.personalContext.myHandSimulation.win != 1)
    {
        return 0;
    }

    if (ctx.personalContext.myHandSimulation.win == 1 || (ctx.personalContext.myHandSimulation.winRanged == 1 &&
                                                          ctx.commonContext.bettingContext.turnBetsOrRaisesNumber < 3))
    {
        return ctx.commonContext.bettingContext.pot * 0.6;
    }

    if (ctx.personalContext.myHandSimulation.winRanged * 100 < ctx.commonContext.bettingContext.potOdd &&
        ctx.personalContext.myHandSimulation.winRanged < 0.94)
    {
        return 0;
    }

    if (ctx.personalContext.myHandSimulation.win == 1 || (ctx.personalContext.myHandSimulation.winRanged == 1 &&
                                                          ctx.commonContext.bettingContext.turnBetsOrRaisesNumber < 3))
    {
        return ctx.commonContext.bettingContext.pot * 0.6;
    }

    if (ctx.personalContext.myHandSimulation.winRanged * 100 < ctx.commonContext.bettingContext.potOdd &&
        ctx.personalContext.myHandSimulation.winRanged < 0.94)
    {
        return 0;
    }

    if (ctx.personalContext.myHandSimulation.winRanged > 0.6 && ctx.personalContext.myHandSimulation.win > 0.6 &&
        ctx.commonContext.bettingContext.turnBetsOrRaisesNumber == 1 &&
        ctx.commonContext.bettingContext.flopBetsOrRaisesNumber < 2 &&
        ctx.commonContext.playersContext.actingPlayersList->size() < 3)
    {
        return ctx.commonContext.bettingContext.pot * 0.6;
    }

    if (ctx.personalContext.myHandSimulation.winRanged > 0.9 && ctx.personalContext.myHandSimulation.win > 0.9 &&
        ctx.commonContext.bettingContext.turnBetsOrRaisesNumber < 4)
    {
        return ctx.commonContext.bettingContext.pot * 0.6;
    }

    return 0;
}

int ManiacBotStrategy::riverShouldBet(const CurrentHandContext& ctx)
{

    if (ctx.commonContext.bettingContext.riverBetsOrRaisesNumber > 0)
    {
        return 0;
    }

    // blocking bet if my chances to win are weak, but not ridiculous
    if (!ctx.personalContext.hasPosition && ctx.personalContext.myHandSimulation.winRanged < 0.7 &&
        ctx.personalContext.myHandSimulation.winRanged > 0.4 && ctx.personalContext.myHandSimulation.winSd > 0.4)
    {
        int rand = 0;
        myServices->randomizer().getRand(1, 2, 1, &rand);
        if (rand == 1)
        {
            return ctx.commonContext.bettingContext.pot * 0.33;
        }
    }

    // bluff if no chance to win, and if I was the agressor on the turn
    if (ctx.personalContext.actions.turnIsAggressor)
    {

        if (ctx.personalContext.myHandSimulation.winRanged < .2 && ctx.personalContext.myHandSimulation.winSd > 0.3 &&
            ctx.commonContext.playersContext.actingPlayersList->size() < 4 &&
            ctx.personalContext.cash >= ctx.commonContext.bettingContext.pot && isPossibleToBluff(ctx))
        {

            int rand = 0;
            myServices->randomizer().getRand(1, 4, 1, &rand);
            if (rand == 1)
            {
                return ctx.commonContext.bettingContext.pot * 0.8;
            }
        }
    }

    int rand = 0;
    myServices->randomizer().getRand(40, 90, 1, &rand);
    float coeff = (float) rand / (float) 100;

    if (ctx.personalContext.myHandSimulation.winSd > .9 ||
        (ctx.personalContext.hasPosition && ctx.personalContext.myHandSimulation.winSd > .85))
    {
        int rand = 0;
        myServices->randomizer().getRand(1, 5, 1, &rand);
        if (rand != 1 || ctx.personalContext.hasPosition)
        {
            return ctx.commonContext.bettingContext.pot * coeff;
        }
    }
    if (ctx.personalContext.myHandSimulation.winSd > 0.5 &&
        (ctx.personalContext.myHandSimulation.winRanged > .8 ||
         (ctx.personalContext.hasPosition && ctx.personalContext.myHandSimulation.winRanged > .7)))
    {
        int rand = 0;
        myServices->randomizer().getRand(1, 3, 1, &rand);
        if (rand == 1 || ctx.personalContext.hasPosition)
        {
            return ctx.commonContext.bettingContext.pot * coeff;
        }
    }
    return 0;
}

bool ManiacBotStrategy::riverShouldCall(const CurrentHandContext& ctx)
{
    const int nbRaises = ctx.commonContext.bettingContext.riverBetsOrRaisesNumber;

    if (nbRaises == 0)
    {
        return false;
    }

    if (ctx.personalContext.myHandSimulation.win > .95 && ctx.personalContext.myHandSimulation.winSd > 0.5)
    {
        return true;
    }

    if (ctx.personalContext.myHandSimulation.winRanged * 100 < ctx.commonContext.bettingContext.potOdd)
    {
        return false;
    }

    if (ctx.personalContext.myHandSimulation.winRanged < .3 && ctx.personalContext.myHandSimulation.winSd < 0.97)
    {
        return false;
    }

    // if hazardous call may cost me my stack, don't call even with good odds
    if (ctx.commonContext.bettingContext.potOdd > 10 && ctx.personalContext.myHandSimulation.winRanged < .4 &&
        ctx.personalContext.myHandSimulation.winSd < 0.97 &&
        ctx.commonContext.bettingContext.highestBetAmount >=
            ctx.personalContext.cash + ctx.personalContext.totalBetAmount &&
        ctx.personalContext.m > 8)
    {

        return false;
    }
    return true;
}

int ManiacBotStrategy::riverShouldRaise(const CurrentHandContext& ctx)
{

    if (ctx.commonContext.bettingContext.riverBetsOrRaisesNumber == 0)
    {
        return 0;
    }

    //  TODO : analyze previous actions, and determine if we must bet for value, without the nuts
    if (ctx.commonContext.bettingContext.riverBetsOrRaisesNumber < 3 &&
        ctx.personalContext.myHandSimulation.winRanged > .98 && ctx.personalContext.myHandSimulation.winSd > 0.5)
    {
        return ctx.commonContext.bettingContext.pot;
    }

    if (ctx.commonContext.bettingContext.riverBetsOrRaisesNumber < 2 &&
        ctx.personalContext.myHandSimulation.winRanged * 100 > ctx.commonContext.bettingContext.potOdd &&
        ctx.personalContext.myHandSimulation.winRanged > 0.9)
    {
        return ctx.commonContext.bettingContext.pot * 0.6;
    }

    return 0;
}
} // namespace pkt::core::player
