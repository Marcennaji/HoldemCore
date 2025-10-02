// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#include <core/player/strategy/LooseAggressiveBotStrategy.h>

#include <core/engine/hand/HandEvaluator.h>

#include <core/engine/model/EngineError.h>
#include <core/engine/model/Ranges.h>
#include <core/player/Helpers.h>
#include <core/player/strategy/CurrentHandContext.h>
#include <core/player/strategy/PokerMath.h>  // Phase 1-4 utilities
#include "Exception.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

namespace pkt::core::player
{

using namespace std;

LooseAggressiveBotStrategy::LooseAggressiveBotStrategy()
{
    // initialize utg starting range, in a full table
    ensureServicesInitialized();
    int utgFullTableRange = PokerMath::getRandomUTGRange(m_services, 13, 15);
    initializeRanges(48, utgFullTableRange);
}

LooseAggressiveBotStrategy::LooseAggressiveBotStrategy(std::shared_ptr<pkt::core::ServiceContainer> serviceContainer)
    : BotStrategyBase(serviceContainer)
{

    // initialize utg starting range, in a full table
    int utgFullTableRange = PokerMath::getRandomUTGRange(m_services, 13, 15);
    initializeRanges(48, utgFullTableRange);
}

LooseAggressiveBotStrategy::~LooseAggressiveBotStrategy() = default;

bool LooseAggressiveBotStrategy::preflopCouldCall(const CurrentHandContext& ctx)
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

        m_services->logger().verbose("\t\tLAG adding high pairs to the initial calling range.");
        stringCallingRange += HIGH_PAIRS;
    }

    std::shared_ptr<Player> lastRaiser = ctx.commonContext.playersContext.preflopLastRaiser;

    if (ctx.commonContext.bettingContext.preflopRaisesNumber < 2 &&
        ctx.personalContext.cash >= ctx.commonContext.bettingContext.pot * 10 && lastRaiser != nullptr &&
        lastRaiser->getCash() >= ctx.commonContext.bettingContext.pot * 10 &&
        !ctx.commonContext.bettingContext.isPreflopBigBet)
    {

        m_services->logger().verbose(
            "\t\tLAG adding high suited connectors, high suited aces and pairs to the initial calling range.");
        stringCallingRange += HIGH_SUITED_CONNECTORS;
        stringCallingRange += HIGH_SUITED_ACES;
        stringCallingRange += PAIRS;

        if (ctx.commonContext.playersContext.actingPlayersList->size() > 2 &&
            ctx.commonContext.bettingContext.preflopRaisesNumber + ctx.commonContext.bettingContext.preflopCallsNumber >
                1 &&
            ctx.personalContext.position >= Late)
        {
            stringCallingRange += SUITED_CONNECTORS;
            stringCallingRange += SUITED_ONE_GAPED;
            stringCallingRange += SUITED_TWO_GAPED;
            m_services->logger().verbose(
                "\t\tLAG adding suited connectors, suited one-gaped and suited two-gaped to the initial "
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

        if (PokerMath::shouldDefendAgainst3Bet(m_services))
        {

            stringCallingRange += HIGH_SUITED_CONNECTORS;
            stringCallingRange += HIGH_SUITED_ACES;
            stringCallingRange += PAIRS;

            m_services->logger().verbose(
                "\t\tLAG defending against 3-bet : adding high suited connectors, high suited aces and pairs to "
                "the initial calling range.");
        }
    }
    m_services->logger().verbose("\t\tLAG final calling range : " + stringCallingRange);

    return isCardsInRange(ctx.personalContext.holeCards, stringCallingRange);
}

int LooseAggressiveBotStrategy::preflopCouldRaise(const CurrentHandContext& ctx)
{
    float raisingRange = getPreflopRangeCalculator()->calculatePreflopRaisingRange(ctx);

    if (raisingRange == -1)
    {
        return 0; // never raise : call or fold
    }

    if (ctx.commonContext.bettingContext.preflopRaisesNumber > 2)
    {
        return 0; // never 5-bet : call or fold
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

    m_services->logger().verbose("Raising range : " + stringRaisingRange);

    // determine when to 3-bet without a real hand
    bool speculativeHandedAdded = false;

    if (ctx.commonContext.bettingContext.preflopRaisesNumber == 1)
    {
        assert(ctx.commonContext.playersContext.preflopLastRaiser != nullptr);
        PreflopStatistics raiserStats = ctx.commonContext.playersContext.preflopLastRaiser
                                            ->getStatistics(ctx.commonContext.playersContext.nbPlayers)
                                            .preflopStatistics;

        if (!isCardsInRange(ctx.personalContext.holeCards, stringRaisingRange) && ctx.personalContext.m > 20 &&
            ctx.personalContext.cash > ctx.commonContext.bettingContext.highestBetAmount * 20 &&
            ctx.personalContext.position > Middle && raiserStats.hands > MIN_HANDS_STATISTICS_ACCURATE &&
            ctx.personalContext.position > ctx.commonContext.playersContext.preflopLastRaiser->getPosition() &&
            ctx.commonContext.playersContext.preflopLastRaiser->getCash() >
                ctx.commonContext.bettingContext.highestBetAmount * 10 &&
            !ctx.commonContext.bettingContext.isPreflopBigBet &&
            ctx.commonContext.bettingContext.preflopCallsNumber < 2)
        {

            if (isPossibleToBluff(ctx) && ctx.personalContext.position > Late &&
                !isCardsInRange(ctx.personalContext.holeCards, ACES + BROADWAYS))
            {

                speculativeHandedAdded = true;
                m_services->logger().verbose("\t\tLAG trying to steal this bet");
            }
            else
            {
                if (isCardsInRange(ctx.personalContext.holeCards,
                                   LOW_PAIRS + CONNECTORS + SUITED_ONE_GAPED + SUITED_TWO_GAPED) &&
                    raiserStats.getPreflopCallthreeBetsFrequency() < 30)
                {

                    speculativeHandedAdded = true;
                    m_services->logger().verbose("\t\tLAG adding this speculative hand to our initial raising range");
                }
                else
                {
                    if (!isCardsInRange(ctx.personalContext.holeCards, PAIRS + ACES + BROADWAYS) &&
                        raiserStats.getPreflopCallthreeBetsFrequency() < 30)
                    {

                        if (PokerMath::shouldAddSpeculativeHand(m_services))
                        {
                            speculativeHandedAdded = true;
                            m_services->logger().verbose("\t\tLAG adding this junk hand to our initial raising range");
                        }
                    }
                }
            }
        }
    }
    // determine when to 4-bet without a real hand
    if (!speculativeHandedAdded && ctx.commonContext.bettingContext.preflopRaisesNumber == 2)
    {
        assert(ctx.commonContext.playersContext.preflopLastRaiser != nullptr);
        PreflopStatistics raiserStats = ctx.commonContext.playersContext.preflopLastRaiser
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

            if (isPossibleToBluff(ctx) && ctx.personalContext.position > Late &&
                isCardsInRange(ctx.personalContext.holeCards, HIGH_SUITED_CONNECTORS) &&
                raiserStats.getPreflop3Bet() > 8)
            {

                speculativeHandedAdded = true;
                m_services->logger().verbose("\t\tLAG adding this speculative hand to our initial raising range");
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

        if (PokerMath::shouldHideHandStrength(m_services))
        {
            m_services->logger().verbose("\t\twon't raise, to hide the hand strength");
            m_couldCall = true;
            return 0;
        }
    }

    return computePreflopRaiseAmount(ctx);
}

int LooseAggressiveBotStrategy::flopCouldBet(const CurrentHandContext& ctx)
{

    if (shouldPotControl(ctx))
    {
        return 0;
    }

    // donk bets :
    if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber > 0 &&
        ctx.commonContext.playersContext.preflopLastRaiser != nullptr &&
        ctx.commonContext.playersContext.preflopLastRaiser->getId() != ctx.personalContext.id)
    {
        if (ctx.commonContext.playersContext.preflopLastRaiser->getPosition() > ctx.personalContext.position)
        {

            if (getDrawingProbability(ctx.personalContext.postFlopAnalysisFlags) > 25)
            {
                if (PokerMath::shouldBluffFrequently(m_services))
                {
                    return PokerMath::calculateValueBetSize(ctx);
                }
            }

            if ((ctx.personalContext.postFlopAnalysisFlags.isTwoPair ||
                 ctx.personalContext.postFlopAnalysisFlags.isTrips ||
                 ctx.personalContext.postFlopAnalysisFlags.isStraight) &&
                ctx.personalContext.postFlopAnalysisFlags.isFlushDrawPossible)
            {
                return PokerMath::calculateValueBetSize(ctx);
            }

            // if the flop is dry, try to get the pot
            if (ctx.commonContext.playersContext.nbPlayers < 3 && isPossibleToBluff(ctx) &&
                getBoardCardsHigherThan(ctx.commonContext.stringBoard, "Jh") < 2 &&
                getBoardCardsHigherThan(ctx.commonContext.stringBoard, "Kh") == 0 &&
                !ctx.personalContext.postFlopAnalysisFlags.isFlushDrawPossible)
            {

                if (PokerMath::shouldBluffFrequently(m_services))
                {
                    return PokerMath::calculateBluffBetSize(ctx);
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
    if ((ctx.personalContext.m_handSimulation.winRanged > 0.5 || ctx.personalContext.m_handSimulation.win > 0.9) &&
        ctx.personalContext.m_handSimulation.win > 0.5)
    {

        // always bet if my hand will lose a lot of its value on next betting rounds
        if (ctx.personalContext.m_handSimulation.winRanged - ctx.personalContext.m_handSimulation.winSd > 0.1)
        {
            return ctx.commonContext.bettingContext.pot;
        }

        if (PokerMath::shouldBluffRarely(m_services) && !ctx.personalContext.hasPosition)
        {
            return 0; // may check-raise or check-call
        }

        // if no raise preflop, or if more than 1 opponent
        if (ctx.commonContext.bettingContext.preflopRaisesNumber == 0 ||
            ctx.commonContext.playersContext.actingPlayersList->size() > 2)
        {

            if (ctx.commonContext.playersContext.actingPlayersList->size() < 4)
            {
                return PokerMath::calculateBluffBetSize(ctx);
            }
            else
            {
                return ctx.commonContext.bettingContext.pot * 1.2;
            }
        }

        // if i have raised preflop, bet
        if (ctx.commonContext.bettingContext.preflopRaisesNumber > 0 &&
            ctx.commonContext.playersContext.preflopLastRaiser != nullptr &&
            ctx.commonContext.playersContext.preflopLastRaiser->getId() == ctx.personalContext.id)
        {
            if (ctx.commonContext.playersContext.actingPlayersList->size() < 4)
            {
                return PokerMath::calculateBluffBetSize(ctx);
            }
            else
            {
                return ctx.commonContext.bettingContext.pot;
            }
        }
    }
    else
    {

        ///////////  if bad flop for me

        // if there was a lot of action preflop, and i was not the last raiser : don't bet
        if (ctx.commonContext.bettingContext.preflopRaisesNumber > 2 &&
            ctx.commonContext.playersContext.preflopLastRaiser != nullptr &&
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

            if (ctx.personalContext.m_handSimulation.winRanged > 0.2)
            {

                return PokerMath::calculateBluffBetSize(ctx);
            }
        }
    }

    return 0;
}
bool LooseAggressiveBotStrategy::flopCouldCall(const CurrentHandContext& ctx)
{

    if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber == 0)
    {
        return false;
    }

    if (isDrawingProbOk(ctx.personalContext.postFlopAnalysisFlags, ctx.commonContext.bettingContext.potOdd))
    {
        return true;
    }

    if (ctx.personalContext.m_handSimulation.winRanged == 1 && ctx.personalContext.m_handSimulation.win > 0.5)
    {
        return true;
    }

    if (PokerMath::hasInsufficientEquityForCall(ctx, 0.9f) &&
        ctx.personalContext.m_handSimulation.win < 0.92)
    {
        return false;
    }

    if (ctx.personalContext.m_handSimulation.winRanged < 0.25 && ctx.personalContext.m_handSimulation.win < 0.9)
    {
        return false;
    }

    return true;
}

int LooseAggressiveBotStrategy::flopCouldRaise(const CurrentHandContext& ctx)
{

    const int nbRaises = ctx.commonContext.bettingContext.flopBetsOrRaisesNumber;

    if (nbRaises == 0)
    {
        if (shouldPotControl(ctx))
        {
            return 0;
        }
    }

    // if (nbRaises == 1 && m_flopHandSimulation.win < 0.90)
    // return false;

    if (nbRaises == 2 && ctx.personalContext.m_handSimulation.win < 0.95)
    {
        return 0;
    }

    if (nbRaises == 3 && ctx.personalContext.m_handSimulation.win < 0.98)
    {
        return 0;
    }

    if (nbRaises > 3 && ctx.personalContext.m_handSimulation.win != 1)
    {
        return 0;
    }

    if ((isDrawingProbOk(ctx.personalContext.postFlopAnalysisFlags, ctx.commonContext.bettingContext.potOdd) ||
         ctx.personalContext.hasPosition) &&
        ctx.commonContext.playersContext.actingPlayersList->size() == 2 &&
        !(ctx.personalContext.m_handSimulation.winRanged * 100 < ctx.commonContext.bettingContext.potOdd) &&
        isPossibleToBluff(ctx) && nbRaises < 2)
    {

        if (PokerMath::shouldBluffOccasionally(m_services))
        {
            return ctx.commonContext.bettingContext.pot;
        }
    }

    if (PokerMath::hasInsufficientEquityForCall(ctx))  // Was: winRanged * 100 < potOdd
    {

        if (ctx.commonContext.bettingContext.potOdd < 30 &&
            ctx.commonContext.playersContext.actingPlayersList->size() < 4)
        {

            if (PokerMath::shouldAddSpeculativeHand(m_services) && ctx.personalContext.m_handSimulation.winRanged > 0.3 &&
                ctx.personalContext.m_handSimulation.win > 0.5)
            {
                return ctx.commonContext.bettingContext.pot;
                ;
            }
        }
        return 0;
    }

    if (ctx.personalContext.m_handSimulation.winRanged > 0.85 && ctx.personalContext.m_handSimulation.win > 0.5 &&
        nbRaises < 3)
    {
        return ctx.commonContext.bettingContext.pot;
    }
    if (ctx.personalContext.m_handSimulation.winRanged > 0.7 && ctx.personalContext.m_handSimulation.win > 0.5 &&
        nbRaises < 2)
    {
        return ctx.commonContext.bettingContext.pot;
    }

    return 0;
}

int LooseAggressiveBotStrategy::turnCouldBet(const CurrentHandContext& ctx)
{

    const int pot = ctx.commonContext.bettingContext.pot + ctx.commonContext.bettingContext.sets;
    const int nbRaises = ctx.commonContext.bettingContext.turnBetsOrRaisesNumber;

    if (nbRaises > 0)
    {
        if (shouldPotControl(ctx))
        {
            return 0;
        }
    }

    if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber > 1 && !ctx.personalContext.actions.flopIsAggressor &&
        ctx.personalContext.m_handSimulation.winRanged < 0.8 && ctx.personalContext.m_handSimulation.win < 0.9)
    {
        return 0;
    }

    if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber == 0 && ctx.personalContext.hasPosition &&
        ctx.commonContext.playersContext.actingPlayersList->size() < 4 &&
        getDrawingProbability(ctx.personalContext.postFlopAnalysisFlags) < 15 && ctx.personalContext.cash > pot * 4)
    {
        return PokerMath::calculateValueBetSize(ctx);
    }

    if (ctx.personalContext.m_handSimulation.winRanged < 0.5 && ctx.personalContext.m_handSimulation.win < 0.9 &&
        !ctx.personalContext.hasPosition)
    {
        return 0;
    }

    if (ctx.personalContext.m_handSimulation.winRanged > 0.5 && ctx.personalContext.m_handSimulation.win > 0.5 &&
        ctx.personalContext.hasPosition)
    {
        return PokerMath::calculateValueBetSize(ctx);
    }

    if (getDrawingProbability(ctx.personalContext.postFlopAnalysisFlags) > 20 && !ctx.personalContext.hasPosition)
    {
        if (PokerMath::shouldBluffFrequently(m_services))
        {
            return PokerMath::calculateValueBetSize(ctx);
        }
    }
    else
    {
        // no draw, not a good hand, but last to speak and nobody has bet
        if (ctx.personalContext.hasPosition && isPossibleToBluff(ctx))
        {
            if (PokerMath::shouldBluffFrequently(m_services))
            {
                return PokerMath::calculateBluffBetSize(ctx);
            }
        }
    }

    return 0;
}

bool LooseAggressiveBotStrategy::turnCouldCall(const CurrentHandContext& ctx)
{
    if (ctx.commonContext.bettingContext.turnBetsOrRaisesNumber == 0)
    {
        return false;
    }

    if (isDrawingProbOk(ctx.personalContext.postFlopAnalysisFlags, ctx.commonContext.bettingContext.potOdd))
    {
        return true;
    }

    TurnStatistics raiserStats =
        ctx.commonContext.playersContext.turnLastRaiser->getStatistics(ctx.commonContext.playersContext.nbPlayers)
            .turnStatistics;

    // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more accurate
    if (raiserStats.hands < MIN_HANDS_STATISTICS_ACCURATE && ctx.commonContext.playersContext.nbPlayers < 10 &&
        ctx.commonContext.playersContext.turnLastRaiser->getStatistics(ctx.commonContext.playersContext.nbPlayers + 1)
                .turnStatistics.hands > MIN_HANDS_STATISTICS_ACCURATE)
    {

        raiserStats = ctx.commonContext.playersContext.turnLastRaiser
                          ->getStatistics(ctx.commonContext.playersContext.nbPlayers + 1)
                          .turnStatistics;
    }

    if (PokerMath::hasInsufficientEquityForCall(ctx) &&
        ctx.personalContext.m_handSimulation.winRanged < 0.94)
    {
        return false;
    }

    if (ctx.commonContext.bettingContext.turnBetsOrRaisesNumber == 2 &&
        ctx.personalContext.m_handSimulation.winRanged < 0.8 && ctx.personalContext.m_handSimulation.win < 0.9)
    {
        if (raiserStats.hands <= MIN_HANDS_STATISTICS_ACCURATE)
        {
            return false;
        }
        if (PokerMath::isOpponentTight(ctx, 20.0f, ctx.commonContext.playersContext.turnLastRaiser))
        {
            return false;
        }
    }
    if (ctx.commonContext.bettingContext.turnBetsOrRaisesNumber > 2 &&
        ctx.personalContext.m_handSimulation.winRanged < 0.9 && ctx.personalContext.m_handSimulation.win < 0.9)
    {
        if (raiserStats.hands <= MIN_HANDS_STATISTICS_ACCURATE)
        {
            return false;
        }
        if (PokerMath::isOpponentTight(ctx, 20.0f, ctx.commonContext.playersContext.turnLastRaiser))
        {
            return false;
        }
    }

    if (ctx.personalContext.m_handSimulation.winRanged < 0.6 && ctx.personalContext.m_handSimulation.win < 0.9 &&
        (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber > 0 || PokerMath::isOpponentPassive(ctx, 30.0f, ctx.commonContext.playersContext.turnLastRaiser)))
    {
        return false;
    }

    if (!ctx.personalContext.actions.preflopIsAggressor && !ctx.personalContext.actions.flopIsAggressor &&
        ctx.personalContext.m_handSimulation.winRanged < 0.8 && ctx.personalContext.m_handSimulation.win < 0.9 &&
        PokerMath::isOpponentPassive(ctx, 30.0f, ctx.commonContext.playersContext.turnLastRaiser) && !ctx.personalContext.hasPosition)
    {
        return false;
    }

    if (ctx.personalContext.m_handSimulation.winRanged < 0.25 && ctx.personalContext.m_handSimulation.win < 0.9)
    {
        return false;
    }

    return true;
}

int LooseAggressiveBotStrategy::turnCouldRaise(const CurrentHandContext& ctx)
{
    if (ctx.commonContext.bettingContext.turnBetsOrRaisesNumber == 0)
    {
        if (shouldPotControl(ctx))
        {
            return 0;
        }
    }

    if (ctx.commonContext.bettingContext.turnBetsOrRaisesNumber == 2 && ctx.personalContext.m_handSimulation.win < 0.95)
    {
        return 0;
    }

    if (ctx.commonContext.bettingContext.turnBetsOrRaisesNumber > 2 && ctx.personalContext.m_handSimulation.win != 1)
    {
        return 0;
    }

    if (ctx.personalContext.m_handSimulation.winRanged > 0.98 && ctx.personalContext.m_handSimulation.win > 0.98 &&
        ctx.personalContext.m_handSimulation.winSd > 0.9)
    {
        if (PokerMath::shouldAddSpeculativeHand(m_services))
        {
            return 0; // very strong hand, slow play, just call
        }
    }

    if (ctx.personalContext.m_handSimulation.win == 1 || (ctx.personalContext.m_handSimulation.winRanged == 1 &&
                                                          ctx.commonContext.bettingContext.turnBetsOrRaisesNumber < 3))
    {
        return PokerMath::calculateValueBetSize(ctx);
    }

    if (PokerMath::hasInsufficientEquityForCall(ctx) &&
        ctx.personalContext.m_handSimulation.winRanged < 0.94)
    {
        return 0;
    }

    if (ctx.personalContext.m_handSimulation.winRanged > 0.7 && ctx.personalContext.m_handSimulation.win > 0.7 &&
        ctx.commonContext.bettingContext.turnBetsOrRaisesNumber == 1 &&
        ctx.commonContext.bettingContext.flopBetsOrRaisesNumber < 2)
    {

        return PokerMath::calculateValueBetSize(ctx);
    }
    if (ctx.personalContext.m_handSimulation.winRanged > 0.94 && ctx.personalContext.m_handSimulation.win > 0.94 &&
        ctx.commonContext.bettingContext.turnBetsOrRaisesNumber < 4)
    {
        return PokerMath::calculateValueBetSize(ctx);
    }

    if ((isDrawingProbOk(ctx.personalContext.postFlopAnalysisFlags, ctx.commonContext.bettingContext.potOdd) ||
         ctx.personalContext.hasPosition) &&
        ctx.commonContext.playersContext.actingPlayersList->size() == 2 &&
        !(ctx.personalContext.m_handSimulation.winRanged * 100 < ctx.commonContext.bettingContext.potOdd) &&
        isPossibleToBluff(ctx) && ctx.commonContext.bettingContext.turnBetsOrRaisesNumber < 2)
    {

        if (PokerMath::shouldAddSpeculativeHand(m_services))
        {
            return PokerMath::calculateValueBetSize(ctx);
        }
    }
    return 0;
}

int LooseAggressiveBotStrategy::riverCouldBet(const CurrentHandContext& ctx)
{

    if (ctx.commonContext.bettingContext.riverBetsOrRaisesNumber > 0)
    {
        return 0;
    }

    // blocking bet if my chances to win are weak, but not ridiculous
    if (!ctx.personalContext.hasPosition && ctx.personalContext.m_handSimulation.winRanged < 0.7 &&
        ctx.personalContext.m_handSimulation.winRanged > 0.4 && ctx.personalContext.m_handSimulation.winSd > 0.5)
    {
        if (PokerMath::shouldBluffFrequently(m_services))
        {
            return PokerMath::calculateBlockingBetSize(ctx);
        }
    }

    // bluff if no chance to win, and if I was the agressor on the turn
    if (ctx.personalContext.actions.turnIsAggressor)
    {

        if (ctx.personalContext.m_handSimulation.winRanged < .15 && ctx.personalContext.m_handSimulation.winSd > 0.3 &&
            ctx.commonContext.playersContext.actingPlayersList->size() < 4 &&
            ctx.personalContext.cash >= ctx.commonContext.bettingContext.pot && isPossibleToBluff(ctx))
        {

            if (PokerMath::shouldAddSpeculativeHand(m_services))
            {
                return PokerMath::calculateBluffBetSize(ctx);
            }
        }
    }

    float coeff = PokerMath::getRandomBetMultiplier(m_services);

    if (ctx.personalContext.m_handSimulation.winSd > .94 ||
        (ctx.personalContext.hasPosition && ctx.personalContext.m_handSimulation.winSd > .9))
    {
        if (!PokerMath::shouldSlowPlay(ctx, m_services) || ctx.personalContext.hasPosition)
        {
            return ctx.commonContext.bettingContext.pot * coeff;
        }
    }
    if (ctx.personalContext.m_handSimulation.winSd > 0.5 &&
        (ctx.personalContext.m_handSimulation.winRanged > .8 ||
         (ctx.personalContext.hasPosition && ctx.personalContext.m_handSimulation.winRanged > .7)))
    {
        if (PokerMath::shouldBluffOccasionally(m_services) || ctx.personalContext.hasPosition)
        {
            return ctx.commonContext.bettingContext.pot * coeff;
        }
    }
    return 0;
}

bool LooseAggressiveBotStrategy::riverCouldCall(const CurrentHandContext& ctx)
{

    const int nbRaises = ctx.commonContext.bettingContext.riverBetsOrRaisesNumber;

    if (nbRaises == 0)
    {
        return false;
    }

    RiverStatistics raiserStats =
        ctx.commonContext.playersContext.lastVPIPPlayer->getStatistics(ctx.commonContext.playersContext.nbPlayers)
            .riverStatistics;

    // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more accurate
    if (raiserStats.hands < MIN_HANDS_STATISTICS_ACCURATE && ctx.commonContext.playersContext.nbPlayers < 10 &&
        ctx.commonContext.playersContext.lastVPIPPlayer->getStatistics(ctx.commonContext.playersContext.nbPlayers + 1)
                .turnStatistics.hands > MIN_HANDS_STATISTICS_ACCURATE)
    {
        raiserStats = ctx.commonContext.playersContext.lastVPIPPlayer
                          ->getStatistics(ctx.commonContext.playersContext.nbPlayers + 1)
                          .riverStatistics;
    }

    if (ctx.personalContext.m_handSimulation.win > .95)
    {
        return true;
    }

    if (PokerMath::hasInsufficientEquityForCall(ctx) &&
        ctx.personalContext.m_handSimulation.winSd < 0.97)
    {
        return false;
    }

    if (ctx.personalContext.m_handSimulation.winRanged < .3 && ctx.personalContext.m_handSimulation.winSd < 0.97)
    {
        return false;
    }

    // if hazardous call may cost me my stack, don't call even with good odds
    if (ctx.commonContext.bettingContext.potOdd > 10 && ctx.personalContext.m_handSimulation.winRanged < .5 &&
        ctx.personalContext.m_handSimulation.winSd < 0.97 &&
        ctx.commonContext.bettingContext.highestBetAmount >=
            ctx.personalContext.cash + ctx.personalContext.totalBetAmount &&
        ctx.personalContext.m > 8)
    {

        if (raiserStats.hands > MIN_HANDS_STATISTICS_ACCURATE &&
            ctx.commonContext.playersContext.lastVPIPPlayer->getStatistics(ctx.commonContext.playersContext.nbPlayers)
                    .getWentToShowDown() < 50)
        {
            return false;
        }
    }

    // assume that if there was more than 1 player to play after the raiser and he is not a maniac, he shouldn't bluff
    // TODO
    /*if (ctx.commonContext.playersContext.actingPlayersList->size() > 2 &&
    ctx.personalContext.m_handSimulation.winRanged < .6 && ctx.personalContext.m_handSimulation.winSd < 0.97 &&
        (raiserStats.hands > MIN_HANDS_STATISTICS_ACCURATE && raiserStats.getAgressionFactor() < 4 &&
         raiserStats.getAgressionFrequency() < 50))
    {

        PlayerListConstIterator it_c;
        int playersAfterRaiser = 0;

        for (it_c = currentHand->getActingPlayersList()->begin(); it_c != currentHand->getActingPlayersList()->end();
             ++it_c)
        {
            if ((*it_c)->getPosition() > ctx.commonContext.playersContext.lastVPIPPlayer->getPosition())
            {
                playersAfterRaiser++;
            }
        }
        if (playersAfterRaiser > 1)
            return false;
    }*/

    if (raiserStats.hands <= MIN_HANDS_STATISTICS_ACCURATE &&
        ctx.commonContext.bettingContext.potOdd * 1.5 > ctx.personalContext.m_handSimulation.winRanged * 100)
    {
        return false;
    }

    return true;
}

int LooseAggressiveBotStrategy::riverCouldRaise(const CurrentHandContext& ctx)
{

    if (ctx.commonContext.bettingContext.riverBetsOrRaisesNumber == 0)
    {
        return 0;
    }

    //  TODO : analyze previous actions, and determine if we must bet for value, without the nuts
    if (ctx.commonContext.bettingContext.riverBetsOrRaisesNumber < 3 &&
        ctx.personalContext.m_handSimulation.winRanged > .98 && ctx.personalContext.m_handSimulation.winSd > 0.5)
    {
        return PokerMath::calculateBluffBetSize(ctx);
    }

    if (ctx.commonContext.bettingContext.riverBetsOrRaisesNumber < 2 &&
        ctx.personalContext.m_handSimulation.winRanged * 100 > ctx.commonContext.bettingContext.potOdd &&
        ctx.personalContext.m_handSimulation.winRanged > 0.9 && ctx.personalContext.m_handSimulation.winSd > 0.5)
    {
        return PokerMath::calculateValueBetSize(ctx);
    }

    return 0;
}
} // namespace pkt::core::player
