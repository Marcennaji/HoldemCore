// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#include <core/player/strategy/ManiacBotStrategy.h>

#include <core/engine/HandEvaluator.h>

#include <core/engine/model/EngineError.h>
#include <core/engine/model/Ranges.h>
#include <core/interfaces/IHand.h>
#include <core/player/Helpers.h>
#include <core/player/strategy/CurrentHandContext.h>
#include <core/services/GlobalServices.h>
#include "Exception.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

namespace pkt::core::player
{

using namespace std;

ManiacBotStrategy::ManiacBotStrategy() : IBotStrategy()
{
    setStrategyName("Maniac");

    // initialize utg starting range, in a full table
    int utgFullTableRange = 0;
    GlobalServices::instance().randomizer()->getRand(30, 35, 1, &utgFullTableRange);
    initializeRanges(50, utgFullTableRange);
}

ManiacBotStrategy::~ManiacBotStrategy() = default;

bool ManiacBotStrategy::preflopShouldCall(CurrentHandContext& ctx)
{

    float callingRange = getPreflopRangeCalculator()->calculatePreflopCallingRange(ctx);
    if (callingRange == -1)
    {
        return false; // never call : raise or fold
    }

    string stringCallingRange;

    const char** rangesString;

    if (ctx.commonContext.nbPlayers == 2)
    {
        rangesString = TOP_RANGE_2_PLAYERS;
    }
    else if (ctx.commonContext.nbPlayers == 3)
    {
        rangesString = TOP_RANGE_3_PLAYERS;
    }
    else if (ctx.commonContext.nbPlayers == 4)
    {
        rangesString = TOP_RANGE_4_PLAYERS;
    }
    else
    {
        rangesString = TOP_RANGE_MORE_4_PLAYERS;
    }

    stringCallingRange = rangesString[(int) callingRange];

    if (ctx.commonContext.preflopRaisesNumber < 3)
    {

        GlobalServices::instance().logger()->verbose("\t\tManiac adding high pairs to the initial calling range.");
        stringCallingRange += HIGH_PAIRS;
    }

    std::shared_ptr<Player> lastRaiser = ctx.commonContext.preflopLastRaiser;

    if (ctx.commonContext.preflopRaisesNumber < 2 && ctx.perPlayerContext.myCash >= ctx.commonContext.pot * 10 &&
        lastRaiser != nullptr && lastRaiser->getCash() >= ctx.commonContext.pot * 10 &&
        !ctx.commonContext.isPreflopBigBet)
    {

        GlobalServices::instance().logger()->verbose(
            "\t\tManiac adding high suited connectors, high suited aces and pairs to the initial calling range.");
        stringCallingRange += HIGH_SUITED_CONNECTORS;
        stringCallingRange += HIGH_SUITED_ACES;
        stringCallingRange += PAIRS;

        if (ctx.commonContext.nbRunningPlayers > 2 &&
            ctx.commonContext.preflopRaisesNumber + ctx.commonContext.preflopCallsNumber > 1 &&
            ctx.perPlayerContext.myPosition >= MIDDLE)
        {
            stringCallingRange += CONNECTORS;
            stringCallingRange += SUITED_ONE_GAPED;
            stringCallingRange += SUITED_TWO_GAPED;
            GlobalServices::instance().logger()->verbose(
                "\t\tManiac adding suited connectors, suited one-gaped and suited two-gaped to the initial "
                "calling range.");
        }
    }

    // defend against 3bet bluffs :
    if (ctx.commonContext.preflopRaisesNumber == 2 &&
        ctx.perPlayerContext.myCurrentHandActions.getPreflopActions().size() > 0 &&
        ctx.perPlayerContext.myCurrentHandActions.getPreflopActions().back() == ActionType::Raise &&
        ctx.perPlayerContext.myCash >= ctx.commonContext.pot * 10 && lastRaiser != nullptr &&
        lastRaiser->getCash() >= ctx.commonContext.pot * 10 && !ctx.commonContext.isPreflopBigBet)
    {

        int rand = 0;
        GlobalServices::instance().randomizer()->getRand(1, 3, 1, &rand);
        if (rand == 1)
        {

            stringCallingRange += HIGH_SUITED_CONNECTORS;
            stringCallingRange += HIGH_SUITED_ACES;
            stringCallingRange += PAIRS;

            GlobalServices::instance().logger()->verbose(
                "\t\tManiac defending against 3-bet : adding high suited connectors, high suited aces and pairs to "
                "the initial calling range.");
        }
    }
    GlobalServices::instance().logger()->verbose("\t\tManiac final calling range : " + stringCallingRange);

    return isCardsInRange(ctx.perPlayerContext.myCard1, ctx.perPlayerContext.myCard2, stringCallingRange);
}

int ManiacBotStrategy::preflopShouldRaise(CurrentHandContext& ctx)
{

    float raisingRange = getPreflopRangeCalculator()->calculatePreflopRaisingRange(ctx);

    if (raisingRange == -1)
    {
        return 0; // never raise : call or fold
    }

    if (ctx.commonContext.preflopRaisesNumber > 3)
    {
        return 0; // never 6-bet : call or fold
    }

    string stringRaisingRange;

    const char** rangesString;

    if (ctx.commonContext.nbPlayers == 2)
    {
        rangesString = TOP_RANGE_2_PLAYERS;
    }
    else if (ctx.commonContext.nbPlayers == 3)
    {
        rangesString = TOP_RANGE_3_PLAYERS;
    }
    else if (ctx.commonContext.nbPlayers == 4)
    {
        rangesString = TOP_RANGE_4_PLAYERS;
    }
    else
    {
        rangesString = TOP_RANGE_MORE_4_PLAYERS;
    }

    stringRaisingRange = rangesString[(int) raisingRange];

    GlobalServices::instance().logger()->verbose(stringRaisingRange);

    // determine when to 3-bet without a real hand
    bool speculativeHandedAdded = false;

    if (ctx.commonContext.preflopRaisesNumber == 1)
    {
        PreflopStatistics raiserStats =
            ctx.commonContext.preflopLastRaiser->getStatistics(ctx.commonContext.nbPlayers).getPreflopStatistics();

        if (!isCardsInRange(ctx.perPlayerContext.myCard1, ctx.perPlayerContext.myCard2, stringRaisingRange) &&
            ctx.perPlayerContext.myM > 20 && ctx.perPlayerContext.myCash > ctx.commonContext.highestSet * 20 &&
            ctx.perPlayerContext.myPosition > UtgPlusTwo && raiserStats.m_hands > MIN_HANDS_STATISTICS_ACCURATE &&
            ctx.perPlayerContext.myPosition > ctx.commonContext.preflopLastRaiser->getPosition() &&
            ctx.commonContext.preflopLastRaiser->getCash() > ctx.commonContext.highestSet * 10 &&
            !ctx.commonContext.isPreflopBigBet && ctx.commonContext.preflopCallsNumber < 2)
        {
            if (ctx.perPlayerContext.myCanBluff && ctx.perPlayerContext.myPosition > LATE &&
                ctx.commonContext.preflopRaisesNumber == 1 &&
                !isCardsInRange(ctx.perPlayerContext.myCard1, ctx.perPlayerContext.myCard2, ACES + BROADWAYS) &&
                raiserStats.getPreflopCall3BetsFrequency() < 30)
            {

                speculativeHandedAdded = true;
                GlobalServices::instance().logger()->verbose("\t\tManiac trying to steal this bet");
            }
            else
            {
                if (isCardsInRange(ctx.perPlayerContext.myCard1, ctx.perPlayerContext.myCard2,
                                   LOW_PAIRS + SUITED_CONNECTORS + SUITED_ONE_GAPED + SUITED_TWO_GAPED))
                {

                    speculativeHandedAdded = true;
                    GlobalServices::instance().logger()->verbose(
                        "\t\tManiac adding this speculative hand to our initial raising range");
                }
                else
                {
                    if (!isCardsInRange(ctx.perPlayerContext.myCard1, ctx.perPlayerContext.myCard2,
                                        PAIRS + ACES + BROADWAYS) &&
                        raiserStats.getPreflopCall3BetsFrequency() < 30)
                    {

                        int rand = 0;
                        GlobalServices::instance().randomizer()->getRand(1, 3, 1, &rand);
                        if (rand == 1)
                        {
                            speculativeHandedAdded = true;
                            GlobalServices::instance().logger()->verbose(
                                "\t\tManiac adding this junk hand to our initial raising range");
                        }
                    }
                }
            }
        }
    }
    // determine when to 4-bet without a real hand
    if (!speculativeHandedAdded && ctx.commonContext.preflopRaisesNumber == 2)
    {
        PreflopStatistics raiserStats =
            ctx.commonContext.preflopLastRaiser->getStatistics(ctx.commonContext.nbPlayers).getPreflopStatistics();

        if (!isCardsInRange(ctx.perPlayerContext.myCard1, ctx.perPlayerContext.myCard2, stringRaisingRange) &&
            !isCardsInRange(ctx.perPlayerContext.myCard1, ctx.perPlayerContext.myCard2, OFFSUITED_BROADWAYS) &&
            ctx.perPlayerContext.myM > 20 && ctx.perPlayerContext.myCash > ctx.commonContext.highestSet * 60 &&
            ctx.perPlayerContext.myPosition > MiddlePlusOne && raiserStats.m_hands > MIN_HANDS_STATISTICS_ACCURATE &&
            ctx.perPlayerContext.myPosition > ctx.commonContext.preflopLastRaiser->getPosition() &&
            ctx.commonContext.preflopLastRaiser->getCash() > ctx.commonContext.highestSet * 20 &&
            !ctx.commonContext.isPreflopBigBet && ctx.commonContext.preflopCallsNumber < 2)
        {

            if (ctx.perPlayerContext.myCanBluff && ctx.perPlayerContext.myPosition > LATE &&
                raiserStats.getPreflop3Bet() > 8)
            {
                int rand = 0;
                GlobalServices::instance().randomizer()->getRand(1, 5, 1, &rand);
                if (rand == 1)
                {
                    speculativeHandedAdded = true;
                    GlobalServices::instance().logger()->verbose(
                        "\t\tManiac adding this speculative hand to our initial raising range");
                }
            }
        }
    }
    if (!speculativeHandedAdded &&
        !isCardsInRange(ctx.perPlayerContext.myCard1, ctx.perPlayerContext.myCard2, stringRaisingRange))
    {
        return 0;
    }

    // sometimes, just call a single raise instead of raising, even with a strong hand
    // nb. raising range 100 means that I want to steal a bet or BB
    if (!speculativeHandedAdded && ctx.commonContext.preflopCallsNumber == 0 &&
        ctx.commonContext.preflopRaisesNumber == 1 && raisingRange < 100 &&
        !(isCardsInRange(ctx.perPlayerContext.myCard1, ctx.perPlayerContext.myCard2, LOW_PAIRS + MEDIUM_PAIRS) &&
          ctx.commonContext.nbPlayers < 4) &&
        !(isCardsInRange(ctx.perPlayerContext.myCard1, ctx.perPlayerContext.myCard2, HIGH_PAIRS) &&
          ctx.commonContext.preflopCallsNumber > 0) &&
        isCardsInRange(ctx.perPlayerContext.myCard1, ctx.perPlayerContext.myCard2,
                       RangeEstimator::getStringRange(ctx.commonContext.nbPlayers, 4)))
    {

        int rand = 0;
        GlobalServices::instance().randomizer()->getRand(1, 10, 1, &rand);
        if (rand == 1)
        {
            GlobalServices::instance().logger()->verbose("\t\twon't raise, to hide the hand strength");
            myShouldCall = true;
            return 0;
        }
    }

    return computePreflopRaiseAmount(ctx);
}

int ManiacBotStrategy::flopShouldBet(CurrentHandContext& ctx)
{

    if (ctx.commonContext.flopBetsOrRaisesNumber > 0)
    {
        return 0;
    }

    if (shouldPotControl(ctx))
    {
        return 0;
    }

    // donk bets :
    if (ctx.commonContext.flopBetsOrRaisesNumber > 0 && ctx.commonContext.preflopRaisesNumber > 0 &&
        ctx.commonContext.preflopLastRaiser->getId() != ctx.perPlayerContext.myID)
    {
        if (ctx.commonContext.preflopLastRaiser->getPosition() > ctx.perPlayerContext.myPosition)
        {

            if (getDrawingProbability(ctx.perPlayerContext.myPostFlopAnalysisFlags) > 25)
            {
                int rand = 0;
                GlobalServices::instance().randomizer()->getRand(1, 2, 1, &rand);
                if (rand == 1)
                {
                    return ctx.commonContext.pot * 0.6;
                }
            }

            if ((ctx.perPlayerContext.myPostFlopAnalysisFlags.isTwoPair ||
                 ctx.perPlayerContext.myPostFlopAnalysisFlags.isTrips ||
                 ctx.perPlayerContext.myPostFlopAnalysisFlags.isStraight) &&
                ctx.perPlayerContext.myPostFlopAnalysisFlags.isFlushDrawPossible)
            {
                return ctx.commonContext.pot * 0.6;
            }

            // if the flop is dry, try to get the pot
            if (ctx.commonContext.nbPlayers < 3 && ctx.perPlayerContext.myCanBluff &&
                getBoardCardsHigherThan(ctx.commonContext.stringBoard, "Jh") < 2 &&
                getBoardCardsHigherThan(ctx.commonContext.stringBoard, "Kh") == 0 &&
                !ctx.perPlayerContext.myPostFlopAnalysisFlags.isFlushDrawPossible)
            {

                int rand = 0;
                GlobalServices::instance().randomizer()->getRand(1, 2, 1, &rand);
                if (rand == 1)
                {
                    return ctx.commonContext.pot * 0.6;
                }
            }
        }
    }

    // don't bet if in position, and pretty good drawing probs
    if (getDrawingProbability(ctx.perPlayerContext.myPostFlopAnalysisFlags) > 20 && ctx.perPlayerContext.myHavePosition)
    {
        return 0;
    }

    // if pretty good hand
    if (ctx.perPlayerContext.myHandSimulation.winRanged > 0.5 || ctx.perPlayerContext.myHandSimulation.win > 0.9)
    {

        // always bet if my hand will lose a lot of its value on next betting rounds
        if (ctx.perPlayerContext.myHandSimulation.winRanged - ctx.perPlayerContext.myHandSimulation.winSd > 0.1)
        {
            return ctx.commonContext.pot * 0.8;
        }

        // if no raise preflop, or if more than 1 opponent
        if (ctx.commonContext.preflopRaisesNumber == 0 || ctx.commonContext.nbRunningPlayers > 2)
        {

            if (ctx.commonContext.nbRunningPlayers < 4)
            {
                return ctx.commonContext.pot * 0.8;
            }
            else
            {
                return ctx.commonContext.pot * 1.2;
            }
        }

        // if i have raised preflop, bet
        if (ctx.commonContext.preflopRaisesNumber > 0 &&
            ctx.commonContext.preflopLastRaiser->getId() == ctx.perPlayerContext.myID)
        {
            return ctx.commonContext.pot * 0.8;
        }
    }
    else
    {

        ///////////  if bad flop for me

        // if there was a lot of action preflop, and i was not the last raiser : don't bet
        if (ctx.commonContext.preflopRaisesNumber > 2 &&
            ctx.commonContext.preflopLastRaiser->getId() != ctx.perPlayerContext.myID)
        {
            return 0;
        }

        // if I was the last raiser preflop, I may bet with not much
        if (ctx.commonContext.preflopRaisesNumber > 0 &&
            ctx.commonContext.preflopLastRaiser->getId() == ctx.perPlayerContext.myID &&
            ctx.commonContext.nbRunningPlayers < 4 && ctx.perPlayerContext.myCash > ctx.commonContext.pot * 4 &&
            ctx.perPlayerContext.myCanBluff)
        {
            if (ctx.perPlayerContext.myHandSimulation.winRanged > 0.15 &&
                ctx.perPlayerContext.myHandSimulation.win > 0.3)
            {
                return ctx.commonContext.pot * 0.8;
            }
        }
    }

    return 0;
}
bool ManiacBotStrategy::flopShouldCall(CurrentHandContext& ctx)
{

    if (ctx.commonContext.flopBetsOrRaisesNumber == 0)
    {
        return false;
    }

    if (isDrawingProbOk(ctx.perPlayerContext.myPostFlopAnalysisFlags, ctx.commonContext.potOdd))
    {
        return true;
    }

    if (ctx.perPlayerContext.myHandSimulation.winRanged == 1 && ctx.perPlayerContext.myHandSimulation.win > 0.5)
    {
        return true;
    }

    if (ctx.perPlayerContext.myHandSimulation.winRanged * 100 < ctx.commonContext.potOdd * 0.8 &&
        ctx.perPlayerContext.myHandSimulation.win < 0.9)
    {
        return false;
    }

    if (ctx.perPlayerContext.myHandSimulation.winRanged < 0.25)
    {
        return false;
    }

    return true;
}

int ManiacBotStrategy::flopShouldRaise(CurrentHandContext& ctx)
{

    const int nbRaises = ctx.commonContext.flopBetsOrRaisesNumber;

    if (nbRaises == 0)
    {
        return 0;
    }

    if (shouldPotControl(ctx))
    {
        return 0;
    }

    if (nbRaises < 2 && ctx.commonContext.nbRunningPlayers < 4 && ctx.perPlayerContext.myCanBluff &&
        ctx.perPlayerContext.myHandSimulation.winRanged < 0.3 &&
        getBoardCardsHigherThan(ctx.commonContext.stringBoard, "Jh") < 2 &&
        getBoardCardsHigherThan(ctx.commonContext.stringBoard, "Kh") == 0)
    {

        int rand = 0;
        GlobalServices::instance().randomizer()->getRand(1, 3, 1, &rand);
        if (rand == 2)
        {
            return ctx.commonContext.pot * 2;
        }
    }

    if (nbRaises == 2 && ctx.perPlayerContext.myHandSimulation.win < 0.95)
    {
        return 0;
    }

    if (nbRaises == 3 && ctx.perPlayerContext.myHandSimulation.win < 0.98)
    {
        return 0;
    }

    if (nbRaises > 3 && ctx.perPlayerContext.myHandSimulation.win != 1)
    {
        return 0;
    }

    if ((isDrawingProbOk(ctx.perPlayerContext.myPostFlopAnalysisFlags, ctx.commonContext.potOdd) ||
         ctx.perPlayerContext.myHavePosition) &&
        ctx.commonContext.nbRunningPlayers == 2 &&
        !(ctx.perPlayerContext.myHandSimulation.winRanged * 100 < ctx.commonContext.potOdd) &&
        ctx.perPlayerContext.myCanBluff && nbRaises < 2)
    {

        int rand = 0;
        GlobalServices::instance().randomizer()->getRand(1, 2, 1, &rand);
        if (rand == 2)
        {
            return ctx.commonContext.pot;
        }
    }

    if (ctx.perPlayerContext.myHandSimulation.winRanged > 0.9 && ctx.perPlayerContext.myHandSimulation.win > 0.5 &&
        nbRaises < 3)
    {
        return ctx.commonContext.pot;
    }
    if (ctx.perPlayerContext.myHandSimulation.winRanged > 0.8 && ctx.perPlayerContext.myHandSimulation.win > 0.5 &&
        nbRaises < 2)
    {
        return ctx.commonContext.pot;
    }

    return 0;
}

int ManiacBotStrategy::turnShouldBet(CurrentHandContext& ctx)
{

    const int pot = ctx.commonContext.pot + ctx.commonContext.sets;
    const int nbRaises = ctx.commonContext.turnBetsOrRaisesNumber;

    if (nbRaises > 0)
    {
        return 0;
    }

    if (shouldPotControl(ctx))
    {
        return 0;
    }

    if (ctx.commonContext.flopBetsOrRaisesNumber > 1 && !ctx.perPlayerContext.myFlopIsAggressor &&
        ctx.perPlayerContext.myHandSimulation.winRanged < 0.8)
    {
        return 0;
    }

    if (ctx.commonContext.flopBetsOrRaisesNumber == 0 && ctx.perPlayerContext.myHavePosition)
    {
        return pot * 0.8;
    }

    if (ctx.perPlayerContext.myHandSimulation.winRanged < 0.3 && ctx.perPlayerContext.myHandSimulation.win < 0.9 &&
        !ctx.perPlayerContext.myHavePosition)
    {
        return 0;
    }

    if (ctx.perPlayerContext.myHandSimulation.winRanged > 0.4 && ctx.perPlayerContext.myHandSimulation.win > 0.5 &&
        ctx.perPlayerContext.myHavePosition)
    {
        return pot * 0.8;
    }

    if (getDrawingProbability(ctx.perPlayerContext.myPostFlopAnalysisFlags) > 15 &&
        !ctx.perPlayerContext.myHavePosition)
    {
        return pot * 0.8;
    }
    else
    {
        // no draw, not a good hand, but last to speak and nobody has bet
        if (ctx.perPlayerContext.myHavePosition && ctx.perPlayerContext.myCanBluff)
        {
            return pot * 0.8;
        }
    }

    return 0;
}

bool ManiacBotStrategy::turnShouldCall(CurrentHandContext& ctx)
{
    if (ctx.commonContext.turnBetsOrRaisesNumber == 0)
    {
        return false;
    }

    if (isDrawingProbOk(ctx.perPlayerContext.myPostFlopAnalysisFlags, ctx.commonContext.potOdd))
    {
        return true;
    }

    TurnStatistics raiserStats =
        ctx.commonContext.turnLastRaiser->getStatistics(ctx.commonContext.nbPlayers).getTurnStatistics();

    // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more
    // accurate
    if (raiserStats.m_hands < MIN_HANDS_STATISTICS_ACCURATE && ctx.commonContext.nbPlayers < 10 &&
        ctx.commonContext.turnLastRaiser->getStatistics(ctx.commonContext.nbPlayers + 1).getTurnStatistics().m_hands >
            MIN_HANDS_STATISTICS_ACCURATE)
    {

        raiserStats =
            ctx.commonContext.turnLastRaiser->getStatistics(ctx.commonContext.nbPlayers + 1).getTurnStatistics();
    }

    if (ctx.perPlayerContext.myHandSimulation.winRanged * 100 < ctx.commonContext.potOdd &&
        ctx.perPlayerContext.myHandSimulation.winRanged < 0.94)
    {
        return false;
    }

    if (ctx.commonContext.turnBetsOrRaisesNumber == 2 && ctx.perPlayerContext.myHandSimulation.winRanged < 0.8 &&
        ctx.perPlayerContext.myHandSimulation.win < 0.9)
    {
        if (raiserStats.m_hands <= MIN_HANDS_STATISTICS_ACCURATE)
        {
            return false;
        }
        if (raiserStats.getAgressionFrequency() < 20)
        {
            return false;
        }
    }
    if (ctx.commonContext.turnBetsOrRaisesNumber > 2 && ctx.perPlayerContext.myHandSimulation.winRanged < 0.8 &&
        ctx.perPlayerContext.myHandSimulation.win < 0.9)
    {
        if (raiserStats.m_hands <= MIN_HANDS_STATISTICS_ACCURATE)
        {
            return false;
        }
        if (raiserStats.getAgressionFrequency() < 20)
        {
            return false;
        }
    }

    if (ctx.perPlayerContext.myHandSimulation.winRanged < 0.5 &&
        (ctx.commonContext.flopBetsOrRaisesNumber > 0 || raiserStats.getAgressionFrequency() < 30))
    {
        return false;
    }

    if (!ctx.perPlayerContext.myPreflopIsAggressor && !ctx.perPlayerContext.myFlopIsAggressor &&
        ctx.perPlayerContext.myHandSimulation.winRanged < 0.7 && raiserStats.getAgressionFrequency() < 30 &&
        !ctx.perPlayerContext.myHavePosition)
    {
        return false;
    }

    if (ctx.perPlayerContext.myHandSimulation.winRanged < 0.25 && ctx.perPlayerContext.myHandSimulation.win < 0.9)
    {
        return false;
    }

    return true;
}

int ManiacBotStrategy::turnShouldRaise(CurrentHandContext& ctx)
{
    if (ctx.commonContext.turnBetsOrRaisesNumber == 0)
    {
        return 0;
    }

    if (shouldPotControl(ctx))
    {
        return 0;
    }

    if (ctx.commonContext.turnBetsOrRaisesNumber == 1 && ctx.perPlayerContext.myHandSimulation.win < 0.9)
    {
        return 0;
    }

    if (ctx.commonContext.turnBetsOrRaisesNumber == 2 && ctx.perPlayerContext.myHandSimulation.win < 0.94)
    {
        return 0;
    }

    if (ctx.commonContext.turnBetsOrRaisesNumber > 2 && ctx.perPlayerContext.myHandSimulation.win != 1)
    {
        return 0;
    }

    if (ctx.perPlayerContext.myHandSimulation.win == 1 ||
        (ctx.perPlayerContext.myHandSimulation.winRanged == 1 && ctx.commonContext.turnBetsOrRaisesNumber < 3))
    {
        return ctx.commonContext.pot * 0.6;
    }

    if (ctx.perPlayerContext.myHandSimulation.winRanged * 100 < ctx.commonContext.potOdd &&
        ctx.perPlayerContext.myHandSimulation.winRanged < 0.94)
    {
        return 0;
    }

    if (ctx.perPlayerContext.myHandSimulation.win == 1 ||
        (ctx.perPlayerContext.myHandSimulation.winRanged == 1 && ctx.commonContext.turnBetsOrRaisesNumber < 3))
    {
        return ctx.commonContext.pot * 0.6;
    }

    if (ctx.perPlayerContext.myHandSimulation.winRanged * 100 < ctx.commonContext.potOdd &&
        ctx.perPlayerContext.myHandSimulation.winRanged < 0.94)
    {
        return 0;
    }

    if (ctx.perPlayerContext.myHandSimulation.winRanged > 0.6 && ctx.perPlayerContext.myHandSimulation.win > 0.6 &&
        ctx.commonContext.turnBetsOrRaisesNumber == 1 && ctx.commonContext.flopBetsOrRaisesNumber < 2 &&
        ctx.commonContext.nbRunningPlayers < 3)
    {
        return ctx.commonContext.pot * 0.6;
    }

    if (ctx.perPlayerContext.myHandSimulation.winRanged > 0.9 && ctx.perPlayerContext.myHandSimulation.win > 0.9 &&
        ctx.commonContext.turnBetsOrRaisesNumber < 4)
    {
        return ctx.commonContext.pot * 0.6;
    }

    return 0;
}

int ManiacBotStrategy::riverShouldBet(CurrentHandContext& ctx)
{

    if (ctx.commonContext.riverBetsOrRaisesNumber > 0)
    {
        return 0;
    }

    // blocking bet if my chances to win are weak, but not ridiculous
    if (!ctx.perPlayerContext.myHavePosition && ctx.perPlayerContext.myHandSimulation.winRanged < 0.7 &&
        ctx.perPlayerContext.myHandSimulation.winRanged > 0.4 && ctx.perPlayerContext.myHandSimulation.winSd > 0.4)
    {
        int rand = 0;
        GlobalServices::instance().randomizer()->getRand(1, 2, 1, &rand);
        if (rand == 1)
        {
            return ctx.commonContext.pot * 0.33;
        }
    }

    // bluff if no chance to win, and if I was the agressor on the turn
    if (ctx.perPlayerContext.myTurnIsAggressor)
    {

        if (ctx.perPlayerContext.myHandSimulation.winRanged < .2 && ctx.perPlayerContext.myHandSimulation.winSd > 0.3 &&
            ctx.commonContext.nbRunningPlayers < 4 && ctx.perPlayerContext.myCash >= ctx.commonContext.pot &&
            ctx.perPlayerContext.myCanBluff)
        {

            int rand = 0;
            GlobalServices::instance().randomizer()->getRand(1, 4, 1, &rand);
            if (rand == 1)
            {
                return ctx.commonContext.pot * 0.8;
            }
        }
    }

    int rand = 0;
    GlobalServices::instance().randomizer()->getRand(40, 90, 1, &rand);
    float coeff = (float) rand / (float) 100;

    if (ctx.perPlayerContext.myHandSimulation.winSd > .9 ||
        (ctx.perPlayerContext.myHavePosition && ctx.perPlayerContext.myHandSimulation.winSd > .85))
    {
        int rand = 0;
        GlobalServices::instance().randomizer()->getRand(1, 5, 1, &rand);
        if (rand != 1 || ctx.perPlayerContext.myHavePosition)
        {
            return ctx.commonContext.pot * coeff;
        }
    }
    if (ctx.perPlayerContext.myHandSimulation.winSd > 0.5 &&
        (ctx.perPlayerContext.myHandSimulation.winRanged > .8 ||
         (ctx.perPlayerContext.myHavePosition && ctx.perPlayerContext.myHandSimulation.winRanged > .7)))
    {
        int rand = 0;
        GlobalServices::instance().randomizer()->getRand(1, 3, 1, &rand);
        if (rand == 1 || ctx.perPlayerContext.myHavePosition)
        {
            return ctx.commonContext.pot * coeff;
        }
    }
    return 0;
}

bool ManiacBotStrategy::riverShouldCall(CurrentHandContext& ctx)
{
    const int nbRaises = ctx.commonContext.riverBetsOrRaisesNumber;

    if (nbRaises == 0)
    {
        return false;
    }

    if (ctx.perPlayerContext.myHandSimulation.win > .95 && ctx.perPlayerContext.myHandSimulation.winSd > 0.5)
    {
        return true;
    }

    if (ctx.perPlayerContext.myHandSimulation.winRanged * 100 < ctx.commonContext.potOdd)
    {
        return false;
    }

    if (ctx.perPlayerContext.myHandSimulation.winRanged < .3 && ctx.perPlayerContext.myHandSimulation.winSd < 0.97)
    {
        return false;
    }

    // if hazardous call may cost me my stack, don't call even with good odds
    if (ctx.commonContext.potOdd > 10 && ctx.perPlayerContext.myHandSimulation.winRanged < .4 &&
        ctx.perPlayerContext.myHandSimulation.winSd < 0.97 &&
        ctx.commonContext.highestSet >= ctx.perPlayerContext.myCash + ctx.perPlayerContext.mySet &&
        ctx.perPlayerContext.myM > 8)
    {

        return false;
    }
    return true;
}

int ManiacBotStrategy::riverShouldRaise(CurrentHandContext& ctx)
{

    if (ctx.commonContext.riverBetsOrRaisesNumber == 0)
    {
        return 0;
    }

    //  TODO : analyze previous actions, and determine if we must bet for value, without the nuts
    if (ctx.commonContext.riverBetsOrRaisesNumber < 3 && ctx.perPlayerContext.myHandSimulation.winRanged > .98 &&
        ctx.perPlayerContext.myHandSimulation.winSd > 0.5)
    {
        return ctx.commonContext.pot;
    }

    if (ctx.commonContext.riverBetsOrRaisesNumber < 2 &&
        ctx.perPlayerContext.myHandSimulation.winRanged * 100 > ctx.commonContext.potOdd &&
        ctx.perPlayerContext.myHandSimulation.winRanged > 0.9)
    {
        return ctx.commonContext.pot * 0.6;
    }

    return 0;
}
} // namespace pkt::core::player
