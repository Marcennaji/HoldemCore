// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#include <core/player/strategy/ManiacBotStrategy.h>

#include <core/engine/CardsValue.h>
#include <core/engine/Randomizer.h>
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
    Randomizer::GetRand(30, 35, 1, &utgFullTableRange);
    initializeRanges(50, utgFullTableRange);
}

ManiacBotStrategy::~ManiacBotStrategy()
{
}

bool ManiacBotStrategy::preflopShouldCall(CurrentHandContext& ctx, bool deterministic)
{

    float callingRange = getPreflopRangeCalculator()->calculatePreflopCallingRange(ctx);
    if (callingRange == -1)
        return false; // never call : raise or fold

    string stringCallingRange;

    const char** RANGES_STRING;

    if (ctx.nbPlayers == 2)
        RANGES_STRING = TOP_RANGE_2_PLAYERS;
    else if (ctx.nbPlayers == 3)
        RANGES_STRING = TOP_RANGE_3_PLAYERS;
    else if (ctx.nbPlayers == 4)
        RANGES_STRING = TOP_RANGE_4_PLAYERS;
    else
        RANGES_STRING = TOP_RANGE_MORE_4_PLAYERS;

    stringCallingRange = RANGES_STRING[(int) callingRange];

    if (ctx.preflopRaisesNumber < 3)
    {

        GlobalServices::instance().logger()->info("\t\tManiac adding high pairs to the initial calling range.");
        stringCallingRange += HIGH_PAIRS;
    }

    std::shared_ptr<Player> lastRaiser = ctx.preflopLastRaiser;

    if (ctx.preflopRaisesNumber < 2 && ctx.myCash >= ctx.pot * 10 && lastRaiser != NULL &&
        lastRaiser->getCash() >= ctx.pot * 10 && !ctx.isPreflopBigBet)
    {

        GlobalServices::instance().logger()->info(
            "\t\tManiac adding high suited connectors, high suited aces and pairs to the initial calling range.");
        stringCallingRange += HIGH_SUITED_CONNECTORS;
        stringCallingRange += HIGH_SUITED_ACES;
        stringCallingRange += PAIRS;

        if (ctx.nbRunningPlayers > 2 && ctx.preflopRaisesNumber + ctx.preflopCallsNumber > 1 &&
            ctx.myPosition >= MIDDLE)
        {
            stringCallingRange += CONNECTORS;
            stringCallingRange += SUITED_ONE_GAPED;
            stringCallingRange += SUITED_TWO_GAPED;
            GlobalServices::instance().logger()->info(
                "\t\tManiac adding suited connectors, suited one-gaped and suited two-gaped to the initial "
                "calling range.");
        }
    }

    // defend against 3bet bluffs :
    if (ctx.preflopRaisesNumber == 2 && ctx.myCurrentHandActions.getPreflopActions().size() > 0 &&
        ctx.myCurrentHandActions.getPreflopActions().back() == PlayerActionRaise && ctx.myCash >= ctx.pot * 10 &&
        lastRaiser != NULL && lastRaiser->getCash() >= ctx.pot * 10 && !ctx.isPreflopBigBet)
    {

        int rand = 0;
        Randomizer::GetRand(1, 3, 1, &rand);
        if (rand == 1)
        {

            stringCallingRange += HIGH_SUITED_CONNECTORS;
            stringCallingRange += HIGH_SUITED_ACES;
            stringCallingRange += PAIRS;

            GlobalServices::instance().logger()->info(
                "\t\tManiac defending against 3-bet : adding high suited connectors, high suited aces and pairs to "
                "the initial calling range.");
        }
    }
    GlobalServices::instance().logger()->info("\t\tManiac final calling range : " + stringCallingRange);

    return isCardsInRange(ctx.myCard1, ctx.myCard2, stringCallingRange);
}

int ManiacBotStrategy::preflopShouldRaise(CurrentHandContext& ctx, bool deterministic)
{

    float raisingRange = getPreflopRangeCalculator()->calculatePreflopRaisingRange(ctx);

    if (raisingRange == -1)
        return 0; // never raise : call or fold

    if (ctx.preflopRaisesNumber > 3)
        return 0; // never 6-bet : call or fold

    string stringRaisingRange;

    const char** RANGES_STRING;

    if (ctx.nbPlayers == 2)
        RANGES_STRING = TOP_RANGE_2_PLAYERS;
    else if (ctx.nbPlayers == 3)
        RANGES_STRING = TOP_RANGE_3_PLAYERS;
    else if (ctx.nbPlayers == 4)
        RANGES_STRING = TOP_RANGE_4_PLAYERS;
    else
        RANGES_STRING = TOP_RANGE_MORE_4_PLAYERS;

    stringRaisingRange = RANGES_STRING[(int) raisingRange];

    GlobalServices::instance().logger()->info(stringRaisingRange);

    // determine when to 3-bet without a real hand
    bool speculativeHandedAdded = false;

    if (ctx.preflopRaisesNumber == 1)
    {
        PreflopStatistics raiserStats = ctx.preflopLastRaiser->getStatistics(ctx.nbPlayers).getPreflopStatistics();

        if (!isCardsInRange(ctx.myCard1, ctx.myCard2, stringRaisingRange) && ctx.myM > 20 &&
            ctx.myCash > ctx.highestSet * 20 && ctx.myPosition > UtgPlusTwo &&
            raiserStats.m_hands > MIN_HANDS_STATISTICS_ACCURATE &&
            ctx.myPosition > ctx.preflopLastRaiser->getPosition() &&
            ctx.preflopLastRaiser->getCash() > ctx.highestSet * 10 && !ctx.isPreflopBigBet &&
            ctx.preflopCallsNumber < 2)
        {
            if (ctx.myCanBluff && ctx.myPosition > LATE && ctx.preflopRaisesNumber == 1 &&
                !isCardsInRange(ctx.myCard1, ctx.myCard2, ACES + BROADWAYS) &&
                raiserStats.getPreflopCall3BetsFrequency() < 30)
            {

                speculativeHandedAdded = true;
                GlobalServices::instance().logger()->info("\t\tManiac trying to steal this bet");
            }
            else
            {
                if (isCardsInRange(ctx.myCard1, ctx.myCard2,
                                   LOW_PAIRS + SUITED_CONNECTORS + SUITED_ONE_GAPED + SUITED_TWO_GAPED))
                {

                    speculativeHandedAdded = true;
                    GlobalServices::instance().logger()->info(
                        "\t\tManiac adding this speculative hand to our initial raising range");
                }
                else
                {
                    if (!isCardsInRange(ctx.myCard1, ctx.myCard2, PAIRS + ACES + BROADWAYS) &&
                        raiserStats.getPreflopCall3BetsFrequency() < 30)
                    {

                        int rand = 0;
                        Randomizer::GetRand(1, 3, 1, &rand);
                        if (rand == 1)
                        {
                            speculativeHandedAdded = true;
                            GlobalServices::instance().logger()->info(
                                "\t\tManiac adding this junk hand to our initial raising range");
                        }
                    }
                }
            }
        }
    }
    // determine when to 4-bet without a real hand
    if (!speculativeHandedAdded && ctx.preflopRaisesNumber == 2)
    {
        PreflopStatistics raiserStats = ctx.preflopLastRaiser->getStatistics(ctx.nbPlayers).getPreflopStatistics();

        if (!isCardsInRange(ctx.myCard1, ctx.myCard2, stringRaisingRange) &&
            !isCardsInRange(ctx.myCard1, ctx.myCard2, OFFSUITED_BROADWAYS) && ctx.myM > 20 &&
            ctx.myCash > ctx.highestSet * 60 && ctx.myPosition > MiddlePlusOne &&
            raiserStats.m_hands > MIN_HANDS_STATISTICS_ACCURATE &&
            ctx.myPosition > ctx.preflopLastRaiser->getPosition() &&
            ctx.preflopLastRaiser->getCash() > ctx.highestSet * 20 && !ctx.isPreflopBigBet &&
            ctx.preflopCallsNumber < 2)
        {

            if (ctx.myCanBluff && ctx.myPosition > LATE && raiserStats.getPreflop3Bet() > 8)
            {
                int rand = 0;
                Randomizer::GetRand(1, 5, 1, &rand);
                if (rand == 1)
                {
                    speculativeHandedAdded = true;
                    GlobalServices::instance().logger()->info(
                        "\t\tManiac adding this speculative hand to our initial raising range");
                }
            }
        }
    }
    if (!speculativeHandedAdded && !isCardsInRange(ctx.myCard1, ctx.myCard2, stringRaisingRange))
        return 0;

    // sometimes, just call a single raise instead of raising, even with a strong hand
    // nb. raising range 100 means that I want to steal a bet or BB
    if (!speculativeHandedAdded && ctx.preflopCallsNumber == 0 && ctx.preflopRaisesNumber == 1 && raisingRange < 100 &&
        !(isCardsInRange(ctx.myCard1, ctx.myCard2, LOW_PAIRS + MEDIUM_PAIRS) && ctx.nbPlayers < 4) &&
        !(isCardsInRange(ctx.myCard1, ctx.myCard2, HIGH_PAIRS) && ctx.preflopCallsNumber > 0) &&
        isCardsInRange(ctx.myCard1, ctx.myCard2, RangeEstimator::getStringRange(ctx.nbPlayers, 4)))
    {

        int rand = 0;
        Randomizer::GetRand(1, 10, 1, &rand);
        if (rand == 1)
        {
            GlobalServices::instance().logger()->info("\t\twon't raise, to hide the hand strength");
            myShouldCall = true;
            return 0;
        }
    }

    return computePreflopRaiseAmount(ctx, deterministic);
}

int ManiacBotStrategy::flopShouldBet(CurrentHandContext& ctx, bool deterministic)
{

    if (ctx.flopBetsOrRaisesNumber > 0)
        return 0;

    if (shouldPotControl(ctx, deterministic))
        return 0;

    // donk bets :
    if (ctx.flopBetsOrRaisesNumber > 0 && ctx.preflopRaisesNumber > 0 && ctx.preflopLastRaiser->getID() != ctx.myID)
    {
        if (ctx.preflopLastRaiser->getPosition() > ctx.myPosition)
        {

            if (getDrawingProbability(ctx.myPostFlopState) > 25)
            {
                int rand = 0;
                Randomizer::GetRand(1, 2, 1, &rand);
                if (rand == 1)
                {
                    return ctx.pot * 0.6;
                }
            }

            if ((ctx.myPostFlopState.IsTwoPair || ctx.myPostFlopState.IsTrips || ctx.myPostFlopState.IsStraight) &&
                ctx.myPostFlopState.IsFlushDrawPossible)
            {
                return ctx.pot * 0.6;
            }

            // if the flop is dry, try to get the pot
            if (ctx.nbPlayers < 3 && ctx.myCanBluff && getBoardCardsHigherThan(ctx.stringBoard, "Jh") < 2 &&
                getBoardCardsHigherThan(ctx.stringBoard, "Kh") == 0 && !ctx.myPostFlopState.IsFlushDrawPossible)
            {

                int rand = 0;
                Randomizer::GetRand(1, 2, 1, &rand);
                if (rand == 1)
                {
                    return ctx.pot * 0.6;
                }
            }
        }
    }

    // don't bet if in position, and pretty good drawing probs
    if (getDrawingProbability(ctx.myPostFlopState) > 20 && ctx.myHavePosition)
        return 0;

    // if pretty good hand
    if (ctx.myHandSimulation.winRanged > 0.5 || ctx.myHandSimulation.win > 0.9)
    {

        // always bet if my hand will lose a lot of its value on next betting rounds
        if (ctx.myHandSimulation.winRanged - ctx.myHandSimulation.winSd > 0.1)
        {
            return ctx.pot * 0.8;
        }

        // if no raise preflop, or if more than 1 opponent
        if (ctx.preflopRaisesNumber == 0 || ctx.nbRunningPlayers > 2)
        {

            if (ctx.nbRunningPlayers < 4)
                return ctx.pot * 0.8;
            else
                return ctx.pot * 1.2;
        }

        // if i have raised preflop, bet
        if (ctx.preflopRaisesNumber > 0 && ctx.preflopLastRaiser->getID() == ctx.myID)
        {
            return ctx.pot * 0.8;
        }
    }
    else
    {

        ///////////  if bad flop for me

        // if there was a lot of action preflop, and i was not the last raiser : don't bet
        if (ctx.preflopRaisesNumber > 2 && ctx.preflopLastRaiser->getID() != ctx.myID)
            return 0;

        // if I was the last raiser preflop, I may bet with not much
        if (ctx.preflopRaisesNumber > 0 && ctx.preflopLastRaiser->getID() == ctx.myID && ctx.nbRunningPlayers < 4 &&
            ctx.myCash > ctx.pot * 4 && ctx.myCanBluff)
        {
            if (ctx.myHandSimulation.winRanged > 0.15 && ctx.myHandSimulation.win > 0.3)
            {
                return ctx.pot * 0.8;
            }
        }
    }

    return 0;
}
bool ManiacBotStrategy::flopShouldCall(CurrentHandContext& ctx, bool deterministic)
{

    if (ctx.flopBetsOrRaisesNumber == 0)
        return false;

    if (isDrawingProbOk(ctx.myPostFlopState, ctx.potOdd))
        return true;

    if (ctx.myHandSimulation.winRanged == 1 && ctx.myHandSimulation.win > 0.5)
        return true;

    if (ctx.myHandSimulation.winRanged * 100 < ctx.potOdd * 0.8 && ctx.myHandSimulation.win < 0.9)
        return false;

    if (ctx.myHandSimulation.winRanged < 0.25)
        return false;

    return true;
}

int ManiacBotStrategy::flopShouldRaise(CurrentHandContext& ctx, bool deterministic)
{

    const int nbRaises = ctx.flopBetsOrRaisesNumber;

    if (nbRaises == 0)
        return 0;

    if (shouldPotControl(ctx, deterministic))
        return 0;

    if (nbRaises < 2 && ctx.nbRunningPlayers < 4 && ctx.myCanBluff && ctx.myHandSimulation.winRanged < 0.3 &&
        getBoardCardsHigherThan(ctx.stringBoard, "Jh") < 2 && getBoardCardsHigherThan(ctx.stringBoard, "Kh") == 0)
    {

        int rand = 0;
        Randomizer::GetRand(1, 3, 1, &rand);
        if (rand == 2)
        {
            return ctx.pot * 2;
        }
    }

    if (nbRaises == 2 && ctx.myHandSimulation.win < 0.95)
        return 0;

    if (nbRaises == 3 && ctx.myHandSimulation.win < 0.98)
        return 0;

    if (nbRaises > 3 && ctx.myHandSimulation.win != 1)
        return 0;

    if ((isDrawingProbOk(ctx.myPostFlopState, ctx.potOdd) || ctx.myHavePosition) && ctx.nbRunningPlayers == 2 &&
        !(ctx.myHandSimulation.winRanged * 100 < ctx.potOdd) && ctx.myCanBluff && nbRaises < 2)
    {

        int rand = 0;
        Randomizer::GetRand(1, 2, 1, &rand);
        if (rand == 2)
        {
            return ctx.pot;
        }
    }

    if (ctx.myHandSimulation.winRanged > 0.9 && ctx.myHandSimulation.win > 0.5 && nbRaises < 3)
    {
        return ctx.pot;
    }
    if (ctx.myHandSimulation.winRanged > 0.8 && ctx.myHandSimulation.win > 0.5 && nbRaises < 2)
    {
        return ctx.pot;
    }

    return 0;
}

int ManiacBotStrategy::turnShouldBet(CurrentHandContext& ctx, bool deterministic)
{

    const int pot = ctx.pot + ctx.sets;
    const int nbRaises = ctx.turnBetsOrRaisesNumber;

    if (nbRaises > 0)
        return 0;

    if (shouldPotControl(ctx, deterministic))
        return 0;

    if (ctx.flopBetsOrRaisesNumber > 1 && !ctx.myFlopIsAggressor && ctx.myHandSimulation.winRanged < 0.8)
        return 0;

    if (ctx.flopBetsOrRaisesNumber == 0 && ctx.myHavePosition)
    {
        return pot * 0.8;
    }

    if (ctx.myHandSimulation.winRanged < 0.3 && ctx.myHandSimulation.win < 0.9 && !ctx.myHavePosition)
        return 0;

    if (ctx.myHandSimulation.winRanged > 0.4 && ctx.myHandSimulation.win > 0.5 && ctx.myHavePosition)
    {
        return pot * 0.8;
    }

    if (getDrawingProbability(ctx.myPostFlopState) > 15 && !ctx.myHavePosition)
    {
        return pot * 0.8;
    }
    else
    {
        // no draw, not a good hand, but last to speak and nobody has bet
        if (ctx.myHavePosition && ctx.myCanBluff)
        {
            return pot * 0.8;
        }
    }

    return 0;
}

bool ManiacBotStrategy::turnShouldCall(CurrentHandContext& ctx, bool deterministic)
{
    if (ctx.turnBetsOrRaisesNumber == 0)
        return false;

    if (isDrawingProbOk(ctx.myPostFlopState, ctx.potOdd))
        return true;

    TurnStatistics raiserStats = ctx.turnLastRaiser->getStatistics(ctx.nbPlayers).getTurnStatistics();

    // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more
    // accurate
    if (raiserStats.m_hands < MIN_HANDS_STATISTICS_ACCURATE && ctx.nbPlayers < 10 &&
        ctx.turnLastRaiser->getStatistics(ctx.nbPlayers + 1).getTurnStatistics().m_hands >
            MIN_HANDS_STATISTICS_ACCURATE)

        raiserStats = ctx.turnLastRaiser->getStatistics(ctx.nbPlayers + 1).getTurnStatistics();

    if (ctx.myHandSimulation.winRanged * 100 < ctx.potOdd && ctx.myHandSimulation.winRanged < 0.94)
    {
        return false;
    }

    if (ctx.turnBetsOrRaisesNumber == 2 && ctx.myHandSimulation.winRanged < 0.8 && ctx.myHandSimulation.win < 0.9)
    {
        if (raiserStats.m_hands <= MIN_HANDS_STATISTICS_ACCURATE)
            return false;
        if (raiserStats.getAgressionFrequency() < 20)
            return false;
    }
    if (ctx.turnBetsOrRaisesNumber > 2 && ctx.myHandSimulation.winRanged < 0.8 && ctx.myHandSimulation.win < 0.9)
    {
        if (raiserStats.m_hands <= MIN_HANDS_STATISTICS_ACCURATE)
            return false;
        if (raiserStats.getAgressionFrequency() < 20)
            return false;
    }

    if (ctx.myHandSimulation.winRanged < 0.5 &&
        (ctx.flopBetsOrRaisesNumber > 0 || raiserStats.getAgressionFrequency() < 30))
        return false;

    if (!ctx.myPreflopIsAggressor && !ctx.myFlopIsAggressor && ctx.myHandSimulation.winRanged < 0.7 &&
        raiserStats.getAgressionFrequency() < 30 && !ctx.myHavePosition)
        return false;

    if (ctx.myHandSimulation.winRanged < 0.25 && ctx.myHandSimulation.win < 0.9)
        return false;

    return true;
}

int ManiacBotStrategy::turnShouldRaise(CurrentHandContext& ctx, bool deterministic)
{
    if (ctx.turnBetsOrRaisesNumber == 0)
        return 0;

    if (shouldPotControl(ctx, deterministic))
        return 0;

    if (ctx.turnBetsOrRaisesNumber == 1 && ctx.myHandSimulation.win < 0.9)
        return 0;

    if (ctx.turnBetsOrRaisesNumber == 2 && ctx.myHandSimulation.win < 0.94)
        return 0;

    if (ctx.turnBetsOrRaisesNumber > 2 && ctx.myHandSimulation.win != 1)
        return 0;

    if (ctx.myHandSimulation.win == 1 || (ctx.myHandSimulation.winRanged == 1 && ctx.turnBetsOrRaisesNumber < 3))
    {
        return ctx.pot * 0.6;
    }

    if (ctx.myHandSimulation.winRanged * 100 < ctx.potOdd && ctx.myHandSimulation.winRanged < 0.94)
        return 0;

    if (ctx.myHandSimulation.win == 1 || (ctx.myHandSimulation.winRanged == 1 && ctx.turnBetsOrRaisesNumber < 3))
    {
        return ctx.pot * 0.6;
    }

    if (ctx.myHandSimulation.winRanged * 100 < ctx.potOdd && ctx.myHandSimulation.winRanged < 0.94)
        return 0;

    if (ctx.myHandSimulation.winRanged > 0.6 && ctx.myHandSimulation.win > 0.6 && ctx.turnBetsOrRaisesNumber == 1 &&
        ctx.flopBetsOrRaisesNumber < 2 && ctx.nbRunningPlayers < 3)
    {
        return ctx.pot * 0.6;
    }

    if (ctx.myHandSimulation.winRanged > 0.9 && ctx.myHandSimulation.win > 0.9 && ctx.turnBetsOrRaisesNumber < 4)
    {
        return ctx.pot * 0.6;
    }

    return 0;
}

int ManiacBotStrategy::riverShouldBet(CurrentHandContext& ctx, bool deterministic)
{

    if (ctx.riverBetsOrRaisesNumber > 0)
        return 0;

    // blocking bet if my chances to win are weak, but not ridiculous
    if (!ctx.myHavePosition && ctx.myHandSimulation.winRanged < 0.7 && ctx.myHandSimulation.winRanged > 0.4 &&
        ctx.myHandSimulation.winSd > 0.4)
    {
        int rand = 0;
        Randomizer::GetRand(1, 2, 1, &rand);
        if (rand == 1)
        {
            return ctx.pot * 0.33;
        }
    }

    // bluff if no chance to win, and if I was the agressor on the turn
    if (ctx.myTurnIsAggressor)
    {

        if (ctx.myHandSimulation.winRanged < .2 && ctx.myHandSimulation.winSd > 0.3 && ctx.nbRunningPlayers < 4 &&
            ctx.myCash >= ctx.pot && ctx.myCanBluff)
        {

            int rand = 0;
            Randomizer::GetRand(1, 4, 1, &rand);
            if (rand == 1)
            {
                return ctx.pot * 0.8;
            }
        }
    }

    int rand = 0;
    Randomizer::GetRand(40, 90, 1, &rand);
    float coeff = (float) rand / (float) 100;

    if (ctx.myHandSimulation.winSd > .9 || (ctx.myHavePosition && ctx.myHandSimulation.winSd > .85))
    {
        int rand = 0;
        Randomizer::GetRand(1, 5, 1, &rand);
        if (rand != 1 || ctx.myHavePosition)
        {
            return ctx.pot * coeff;
        }
    }
    if (ctx.myHandSimulation.winSd > 0.5 &&
        (ctx.myHandSimulation.winRanged > .8 || (ctx.myHavePosition && ctx.myHandSimulation.winRanged > .7)))
    {
        int rand = 0;
        Randomizer::GetRand(1, 3, 1, &rand);
        if (rand == 1 || ctx.myHavePosition)
        {
            return ctx.pot * coeff;
        }
    }
    return 0;
}

bool ManiacBotStrategy::riverShouldCall(CurrentHandContext& ctx, bool deterministic)
{
    const int nbRaises = ctx.riverBetsOrRaisesNumber;

    if (nbRaises == 0)
        return false;

    if (ctx.myHandSimulation.win > .95 && ctx.myHandSimulation.winSd > 0.5)
    {
        return true;
    }

    if (ctx.myHandSimulation.winRanged * 100 < ctx.potOdd)
        return false;

    if (ctx.myHandSimulation.winRanged < .3 && ctx.myHandSimulation.winSd < 0.97)
    {
        return false;
    }

    // if hazardous call may cost me my stack, don't call even with good odds
    if (ctx.potOdd > 10 && ctx.myHandSimulation.winRanged < .4 && ctx.myHandSimulation.winSd < 0.97 &&
        ctx.highestSet >= ctx.myCash + ctx.mySet && ctx.myM > 8)
    {

        return false;
    }
    return true;
}

int ManiacBotStrategy::riverShouldRaise(CurrentHandContext& ctx, bool deterministic)
{

    if (ctx.riverBetsOrRaisesNumber == 0)
        return 0;

    //  TODO : analyze previous actions, and determine if we must bet for value, without the nuts
    if (ctx.riverBetsOrRaisesNumber < 3 && ctx.myHandSimulation.winRanged > .98 && ctx.myHandSimulation.winSd > 0.5)
    {
        return ctx.pot;
    }

    if (ctx.riverBetsOrRaisesNumber < 2 && ctx.myHandSimulation.winRanged * 100 > ctx.potOdd &&
        ctx.myHandSimulation.winRanged > 0.9)
    {
        return ctx.pot * 0.6;
    }

    return 0;
}
} // namespace pkt::core::player
