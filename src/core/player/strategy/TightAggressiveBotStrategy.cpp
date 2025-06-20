// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#include <core/player/strategy/TightAggressiveBotStrategy.h>

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

TightAggressiveBotStrategy::TightAggressiveBotStrategy() : IBotStrategy()
{
    setStrategyName("TightAggressive");

    // initialize utg starting range, in a full table
    int utgFullTableRange = 0;
    Randomizer::GetRand(2, 3, 1, &utgFullTableRange);
    initializeRanges(45, utgFullTableRange);
}

TightAggressiveBotStrategy::~TightAggressiveBotStrategy()
{
}

bool TightAggressiveBotStrategy::preflopShouldCall(CurrentHandContext& ctx, bool deterministic)
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

    std::shared_ptr<Player> lastRaiser = ctx.preflopLastRaiser;

    if (ctx.nbRunningPlayers > 2 && ctx.preflopRaisesNumber + ctx.preflopCallsNumber > 1 &&
        ctx.preflopRaisesNumber == 1 && ctx.myPosition >= LATE && ctx.myCash >= ctx.pot * 10 && lastRaiser != NULL &&
        lastRaiser->getCash() >= ctx.pot * 20 && !ctx.isPreflopBigBet)
    {

        stringCallingRange += HIGH_SUITED_CONNECTORS;

        GlobalServices::instance().logger()->info(
            "\t\tTAG adding high suited connectors to the initial calling range.");
    }

    // defend against 3bet bluffs :
    if (ctx.preflopRaisesNumber == 2 && ctx.myCurrentHandActions.getPreflopActions().size() > 0 &&
        ctx.myCurrentHandActions.getPreflopActions().back() == PlayerActionRaise && ctx.myCash >= ctx.pot * 10 &&
        lastRaiser != NULL && lastRaiser->getCash() >= ctx.pot * 10 && !ctx.isPreflopBigBet)
    {

        int rand = 0;
        Randomizer::GetRand(1, 4, 1, &rand);
        if (!deterministic && rand == 1)
        {

            stringCallingRange += HIGH_SUITED_CONNECTORS;
            stringCallingRange += HIGH_SUITED_ACES;
            stringCallingRange += PAIRS;

            GlobalServices::instance().logger()->info(
                "\t\tTAG defending against 3-bet : adding high suited connectors, high suited aces and pairs to "
                "the initial calling range.");
        }
    }
    GlobalServices::instance().logger()->info("\t\tTAG final calling range : " + stringCallingRange);

    return isCardsInRange(ctx.myCard1, ctx.myCard2, stringCallingRange);
}

int TightAggressiveBotStrategy::preflopShouldRaise(CurrentHandContext& ctx, bool deterministic)
{

    float raisingRange = getPreflopRangeCalculator()->calculatePreflopRaisingRange(ctx);

    if (raisingRange == -1)
        return 0; // never raise : call or fold

    if (ctx.preflopRaisesNumber > 1)
        return 0; // never 4-bet : call or fold

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
            ctx.myCash > ctx.highestSet * 20 && ctx.myPosition > MiddlePlusOne &&
            raiserStats.m_hands > MIN_HANDS_STATISTICS_ACCURATE &&
            ctx.myPosition > ctx.preflopLastRaiser->getPosition() &&
            ctx.preflopLastRaiser->getCash() > ctx.highestSet * 10 && !ctx.isPreflopBigBet &&
            ctx.preflopCallsNumber < 2)
        {

            if (ctx.myCanBluff && ctx.myPosition > LATE &&
                !isCardsInRange(ctx.myCard1, ctx.myCard2, ACES + BROADWAYS) &&
                raiserStats.getPreflopCall3BetsFrequency() < 30)
            {

                int rand = 0;
                Randomizer::GetRand(1, 2, 1, &rand);
                if (!deterministic && rand == 2)
                {
                    speculativeHandedAdded = true;
                    GlobalServices::instance().logger()->info("\t\tTAG trying to steal this bet");
                }
            }
            else
            {
                if (isCardsInRange(ctx.myCard1, ctx.myCard2,
                                   LOW_PAIRS + CONNECTORS + SUITED_ONE_GAPED + SUITED_TWO_GAPED) &&
                    raiserStats.getPreflopCall3BetsFrequency() < 30)
                {

                    speculativeHandedAdded = true;
                    GlobalServices::instance().logger()->info(
                        "\t\tTAG adding this speculative hand to our initial raising range");
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
        Randomizer::GetRand(1, 8, 1, &rand);
        if (!deterministic && rand == 1)
        {
            GlobalServices::instance().logger()->info("\t\twon't raise, to hide the hand strength");
            myShouldCall = true;
            return 0;
        }
    }

    return computePreflopRaiseAmount(ctx, deterministic);
}

int TightAggressiveBotStrategy::flopShouldBet(CurrentHandContext& ctx, bool deterministic)
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
                if (!deterministic && rand == 1)
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
                Randomizer::GetRand(1, 3, 1, &rand);
                if (!deterministic && rand == 1)
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
    if (ctx.myHandSimulation.winRanged > 0.6 || ctx.myHandSimulation.win > 0.94)
    {

        // always bet if my hand will lose a lot of its value on next betting rounds
        if (ctx.myHandSimulation.winRanged - ctx.myHandSimulation.winSd > 0.1 && ctx.myHavePosition)
        {
            return ctx.pot;
        }

        int rand = 0;
        Randomizer::GetRand(1, 7, 1, &rand);
        if (!deterministic && rand == 3 && !ctx.myHavePosition && ctx.preflopLastRaiser->getID() != ctx.myID)
            return 0; // may check-raise or check-call

        // if no raise preflop, or if more than 1 opponent
        if (ctx.preflopRaisesNumber == 0 || ctx.nbRunningPlayers > 2)
        {

            if (ctx.nbRunningPlayers < 4)
                return ctx.pot * 0.6;
            else
                return ctx.pot;
        }

        // if i have raised preflop, bet
        if (ctx.preflopRaisesNumber > 0 && ctx.preflopLastRaiser->getID() == ctx.myID)
        {
            if (ctx.nbRunningPlayers < 4)
                return ctx.pot * 0.6;
            else
                return ctx.pot;
        }
    }
    else
    {

        ///////////  if bad flop for me

        // if there was a lot of action preflop, and i was not the last raiser : don't bet
        if (ctx.preflopRaisesNumber > 1 && ctx.preflopLastRaiser->getID() != ctx.myID)
            return 0;

        // if I was the last raiser preflop, I may bet with not much
        if (ctx.preflopLastRaiser->getID() == ctx.myID && ctx.nbRunningPlayers < 4 && ctx.myCash > ctx.pot * 5 &&
            ctx.myCanBluff)
        {

            return ctx.pot * 0.6;
        }
    }

    return 0;
}
bool TightAggressiveBotStrategy::flopShouldCall(CurrentHandContext& ctx, bool deterministic)
{

    if (ctx.flopBetsOrRaisesNumber == 0)
        return false;

    if (isDrawingProbOk(ctx.myPostFlopState, ctx.potOdd))
        return true;

    if (ctx.myHandSimulation.winRanged == 1 && ctx.myHandSimulation.win > 0.5)
        return true;

    if (ctx.myHandSimulation.winRanged * 100 < ctx.potOdd && ctx.myHandSimulation.win < 0.94)
        return false;

    if (ctx.myHandSimulation.winRanged < 0.25 && ctx.myHandSimulation.win < 0.3)
        return false;

    return true;
}

int TightAggressiveBotStrategy::flopShouldRaise(CurrentHandContext& ctx, bool deterministic)
{

    const int nbRaises = ctx.flopBetsOrRaisesNumber;

    if (nbRaises == 0)
        return 0;

    if (shouldPotControl(ctx, deterministic))
        return 0;

    if (nbRaises == 1 && ctx.myHandSimulation.win < 0.90)
        return false;

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
        Randomizer::GetRand(1, 4, 1, &rand);
        if (!deterministic && rand == 2)
        {
            return ctx.pot;
        }
    }

    if (ctx.myHandSimulation.winRanged * 100 < ctx.potOdd)
    {

        if (ctx.potOdd < 30 && ctx.nbRunningPlayers < 4)
        {

            int rand = 0;
            Randomizer::GetRand(1, 6, 1, &rand);
            if (!deterministic && rand == 2 && ctx.myHandSimulation.winRanged > 0.3 && ctx.myHandSimulation.win > 0.3)
            {
                return ctx.pot;
                ;
            }
        }
        return 0;
    }

    if (ctx.myHandSimulation.winRanged > 0.9 && nbRaises < 3)
    {
        return ctx.pot;
    }
    if (ctx.myHandSimulation.winRanged > 0.75 && nbRaises < 2)
    {
        return ctx.pot;
    }

    return 0;
}

int TightAggressiveBotStrategy::turnShouldBet(CurrentHandContext& ctx, bool deterministic)
{

    const int pot = ctx.pot + ctx.sets;
    const int nbRaises = ctx.turnBetsOrRaisesNumber;

    if (nbRaises > 0)
        return 0;

    if (shouldPotControl(ctx, deterministic))
        return 0;

    if (ctx.flopBetsOrRaisesNumber > 1 && !ctx.myFlopIsAggressor && ctx.myHandSimulation.winRanged < 0.75 &&
        ctx.myHandSimulation.win < 0.9)
        return 0;

    if (ctx.flopBetsOrRaisesNumber == 0 && ctx.myHavePosition && ctx.nbRunningPlayers < 4 &&
        getDrawingProbability(ctx.myPostFlopState) < 9 && ctx.myCash > pot * 4)
    {
        int rand = 0;
        Randomizer::GetRand(1, 2, 1, &rand);
        if (!deterministic && rand == 1)
        {
            return pot * 0.6;
        }
    }

    if (ctx.myCash < ctx.pot * 4 && ctx.myHandSimulation.winRanged < 0.6 && ctx.myHandSimulation.win < 0.9)
        return 0;

    if (ctx.myHandSimulation.winRanged < 0.6 && ctx.myHandSimulation.win < 0.9 && !ctx.myHavePosition)
        return 0;

    if (ctx.myHandSimulation.winRanged > 0.5 && ctx.myHandSimulation.win > 0.7 && ctx.myHavePosition)
    {
        return pot * 0.6;
    }

    if (getDrawingProbability(ctx.myPostFlopState) > 20 && !ctx.myHavePosition)
    {
        int rand = 0;
        Randomizer::GetRand(1, 3, 1, &rand);
        if (!deterministic && rand == 1)
        {
            return pot * 0.6;
        }
    }
    else
    {
        // no draw, not a good hand, but last to speak and nobody has bet
        if (ctx.myHavePosition && ctx.myCanBluff)
        {
            int rand = 0;
            Randomizer::GetRand(1, 3, 1, &rand);
            if (!deterministic && rand == 2)
            {
                return pot * 0.6;
            }
        }
    }

    return 0;
}

bool TightAggressiveBotStrategy::turnShouldCall(CurrentHandContext& ctx, bool deterministic)
{
    if (ctx.turnBetsOrRaisesNumber == 0)
        return false;

    if (isDrawingProbOk(ctx.myPostFlopState, ctx.potOdd))
        return true;

    TurnStatistics raiserStats = ctx.turnLastRaiser->getStatistics(ctx.nbPlayers).getTurnStatistics();

    // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more accurate
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
    if (ctx.turnBetsOrRaisesNumber > 2 && ctx.myHandSimulation.winRanged < 0.9 && ctx.myHandSimulation.win < 0.95)
    {
        if (raiserStats.m_hands <= MIN_HANDS_STATISTICS_ACCURATE)
            return false;
        if (raiserStats.getAgressionFrequency() < 20)
            return false;
    }

    if (ctx.myHandSimulation.winRanged < 0.6 && ctx.myHandSimulation.win < 0.95 &&
        (ctx.flopBetsOrRaisesNumber > 0 || raiserStats.getAgressionFrequency() < 30))
        return false;

    if (!ctx.myPreflopIsAggressor && !ctx.myFlopIsAggressor && ctx.myHandSimulation.winRanged < 0.8 &&
        ctx.myHandSimulation.win < 0.95 && raiserStats.getAgressionFrequency() < 30 && !ctx.myHavePosition)
        return false;

    if (ctx.myHandSimulation.winRanged < 0.25 && ctx.myHandSimulation.win < 0.95)
        return false;

    return true;
}

int TightAggressiveBotStrategy::turnShouldRaise(CurrentHandContext& ctx, bool deterministic)
{
    if (ctx.turnBetsOrRaisesNumber == 0)
        return 0;

    if (shouldPotControl(ctx, deterministic))
        return 0;

    if (ctx.turnBetsOrRaisesNumber == 2 && ctx.myHandSimulation.win < 0.98)
        return 0;

    if (ctx.turnBetsOrRaisesNumber > 2 && ctx.myHandSimulation.win != 1)
        return 0;

    if (ctx.myHandSimulation.winRanged > 0.98 && ctx.myHandSimulation.win > 0.98 && ctx.myHandSimulation.winSd > 0.9)
    {
        int rand = 0;
        Randomizer::GetRand(1, 3, 1, &rand);
        if (!deterministic && rand == 1)
            return 0; // very strong hand, slow play, just call
    }

    if (ctx.myHandSimulation.win == 1 || (ctx.myHandSimulation.winRanged == 1 && ctx.turnBetsOrRaisesNumber < 3))
    {
        return ctx.pot * 0.6;
    }

    if (ctx.myHandSimulation.winRanged * 100 < ctx.potOdd && ctx.myHandSimulation.winRanged < 0.94)
        return 0;

    if (ctx.myHandSimulation.winRanged > 0.9 && ctx.myHandSimulation.win > 0.9 && ctx.turnBetsOrRaisesNumber == 1 &&
        ctx.flopBetsOrRaisesNumber < 2)
    {

        return ctx.pot * 0.6;
    }
    if (ctx.myHandSimulation.winRanged > 0.94 && ctx.myHandSimulation.win > 0.94 && ctx.turnBetsOrRaisesNumber < 4)
    {
        return ctx.pot * 0.6;
    }

    return 0;
}

int TightAggressiveBotStrategy::riverShouldBet(CurrentHandContext& ctx, bool deterministic)
{

    if (ctx.riverBetsOrRaisesNumber > 0)
        return 0;

    // blocking bet if my chances to win are weak, but not ridiculous
    if (!ctx.myHavePosition && ctx.myHandSimulation.winRanged < 0.7 && ctx.myHandSimulation.winRanged > 0.4 &&
        ctx.myHandSimulation.winSd > 0.4)
    {
        int rand = 0;
        Randomizer::GetRand(1, 2, 1, &rand);
        if (!deterministic && rand == 1)
        {
            return ctx.pot * 0.33;
        }
    }

    // bluff if no chance to win, and if I was the agressor on the turn
    if (ctx.myTurnIsAggressor)
    {

        if (ctx.myHandSimulation.winRanged < .15 && ctx.myHandSimulation.winSd > 0.3 && ctx.nbRunningPlayers < 4 &&
            ctx.myCash >= ctx.pot && ctx.myCanBluff)
        {

            int rand = 0;
            Randomizer::GetRand(1, 4, 1, &rand);
            if (!deterministic && rand == 1)
            {
                return ctx.pot * 0.8;
            }
        }
    }

    if (ctx.myHandSimulation.winSd < .94 && ctx.turnBetsOrRaisesNumber > 0 && !ctx.myTurnIsAggressor)
        return false;

    int rand = 0;
    Randomizer::GetRand(40, 80, 1, &rand);
    float coeff = (float) rand / (float) 100;

    if (ctx.myHandSimulation.winSd > .94 || (ctx.myHavePosition && ctx.myHandSimulation.winSd > .9))
    {
        int rand = 0;
        Randomizer::GetRand(1, 6, 1, &rand);
        if (!deterministic && rand != 1 || ctx.myHavePosition)
        {
            return ctx.pot * coeff;
        }
    }
    if (ctx.myHandSimulation.winSd > 0.5 &&
        (ctx.myHandSimulation.winRanged > .9 || (ctx.myHavePosition && ctx.myHandSimulation.winRanged > .8)))
    {
        int rand = 0;
        Randomizer::GetRand(1, 3, 1, &rand);
        if (!deterministic && (rand == 1 || ctx.myHavePosition))
        {
            return ctx.pot * coeff;
        }
    }
    if (ctx.myHavePosition && ctx.riverBetsOrRaisesNumber == 0 && ctx.myHandSimulation.winRanged < 0.3 &&
        ctx.turnBetsOrRaisesNumber == 0)
    {
        int rand = 0;
        Randomizer::GetRand(1, 6, 1, &rand);
        if (!deterministic && rand == 1)
        {
            return ctx.pot * coeff;
        }
    }
    if (ctx.myHavePosition && ctx.riverBetsOrRaisesNumber == 0 && ctx.myHandSimulation.winRanged > 0.7 &&
        ctx.turnBetsOrRaisesNumber == 0)
    {
        int rand = 0;
        Randomizer::GetRand(1, 3, 1, &rand);
        if (!deterministic && rand != 1)
        {
            return ctx.pot * coeff;
        }
    }
    return 0;
}

bool TightAggressiveBotStrategy::riverShouldCall(CurrentHandContext& ctx, bool deterministic)
{

    const int nbRaises = ctx.riverBetsOrRaisesNumber;

    if (nbRaises == 0)
        return false;

    RiverStatistics raiserStats = ctx.lastVPIPPlayer->getStatistics(ctx.nbPlayers).getRiverStatistics();

    // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more accurate
    if (raiserStats.m_hands < MIN_HANDS_STATISTICS_ACCURATE && ctx.nbPlayers < 10 &&
        ctx.lastVPIPPlayer->getStatistics(ctx.nbPlayers + 1).getTurnStatistics().m_hands >
            MIN_HANDS_STATISTICS_ACCURATE)
        raiserStats = ctx.lastVPIPPlayer->getStatistics(ctx.nbPlayers + 1).getRiverStatistics();

    if (ctx.myHandSimulation.winRanged * 100 < ctx.potOdd && ctx.myHandSimulation.winRanged < 0.94)
    {
        if (raiserStats.m_hands > MIN_HANDS_STATISTICS_ACCURATE && raiserStats.getAgressionFrequency() < 40)
            return false;
    }

    if (ctx.myHandSimulation.winRanged < .7 && nbRaises == 1)
    {
        if (raiserStats.m_hands > MIN_HANDS_STATISTICS_ACCURATE &&
            ctx.lastVPIPPlayer->getStatistics(ctx.nbPlayers).getWentToShowDown() < 40)
            return false;
    }

    if (ctx.myHandSimulation.winRanged < .9 && nbRaises > 1)
    {
        if (raiserStats.m_hands > MIN_HANDS_STATISTICS_ACCURATE &&
            ctx.lastVPIPPlayer->getStatistics(ctx.nbPlayers).getWentToShowDown() < 40)
            return false;
    }

    // if hazardous call may cost me my stack, don't call even with good odds
    if (ctx.potOdd > 10 && ctx.myHandSimulation.winRanged < .5 && ctx.highestSet >= ctx.myCash + ctx.mySet &&
        ctx.myM > 8)
    {

        if (raiserStats.m_hands > MIN_HANDS_STATISTICS_ACCURATE &&
            ctx.lastVPIPPlayer->getStatistics(ctx.nbPlayers).getWentToShowDown() < 50)
            return false;
    }

    // assume that if there was more than 1 player to play after the raiser and he is not a maniac, he shouldn't bluff
    // TODO
    /*if (ctx.nbRunningPlayers > 2 && ctx.myHandSimulation.winRanged < .6 &&
        (raiserStats.m_hands > MIN_HANDS_STATISTICS_ACCURATE && raiserStats.getAgressionFactor() < 4 &&
         raiserStats.getAgressionFrequency() < 50))
    {

        PlayerListConstIterator it_c;
        int playersAfterRaiser = 0;

        for (it_c = currentHand->getRunningPlayerList()->begin(); it_c != currentHand->getRunningPlayerList()->end();
             ++it_c)
        {
            if ((*it_c)->getPosition() > ctx.lastVPIPPlayer->getPosition())
            {
                playersAfterRaiser++;
            }
        }
        if (playersAfterRaiser > 1)
            return false;
    }*/

    if (raiserStats.m_hands <= MIN_HANDS_STATISTICS_ACCURATE && ctx.potOdd * 1.5 > ctx.myHandSimulation.winRanged * 100)
        return false;

    return true;
}

int TightAggressiveBotStrategy::riverShouldRaise(CurrentHandContext& ctx, bool deterministic)
{

    if (ctx.riverBetsOrRaisesNumber == 0)
        return 0;

    //  TODO : analyze previous actions, and determine if we must bet for value, without the nuts
    if (ctx.riverBetsOrRaisesNumber < 3 && ctx.myHandSimulation.winRanged > .98 && ctx.myHandSimulation.winSd > 0.5)
    {
        return ctx.pot * 0.6;
    }

    if (ctx.riverBetsOrRaisesNumber < 2 && ctx.myHandSimulation.winRanged * 100 > ctx.potOdd &&
        ctx.myHandSimulation.winRanged > 0.9 && ctx.myHandSimulation.winSd > 0.5)
    {
        return ctx.pot * 0.6;
    }

    if (ctx.myHandSimulation.winRanged == 1.0f && ctx.myHandSimulation.winSd == 1.0f &&
        ctx.myHandSimulation.win == 1.0f)
    {
        // if nuts, bet to get value
        return ctx.pot * 0.6;
    }

    return 0;
}
} // namespace pkt::core::player
