#include "IBotStrategy.h"

#include <core/engine/model/Ranges.h>
#include <core/player/Helpers.h>
#include <core/services/GlobalServices.h>
#include "CurrentHandContext.h"

using namespace std;

namespace pkt::core::player
{

void IBotStrategy::initializeRanges(const int utgHeadsUpRange, const int utgFullTableRange)
{
    myPreflopRangeCalculator->initializeRanges(utgHeadsUpRange, utgFullTableRange);
}

int IBotStrategy::computePreflopRaiseAmount(CurrentHandContext& ctx)
{
    int myRaiseAmount = 0;

    const int nbRaises = ctx.commonContext.preflopRaisesNumber;
    const int nbCalls = ctx.commonContext.preflopCallsNumber;
    const int nbPlayers = ctx.commonContext.nbPlayers;

    const int bigBlind = ctx.commonContext.smallBlind * 2;

    if (nbRaises == 0)
    { // first to raise

        myRaiseAmount = (ctx.perPlayerContext.myM > 8 ? 2 * bigBlind : 1.5 * bigBlind);

        if (nbPlayers > 4)
        { // adjust for position
            if (ctx.perPlayerContext.myPosition < MIDDLE)
            {
                myRaiseAmount += bigBlind;
            }
            if (ctx.perPlayerContext.myPosition == BUTTON)
            {
                myRaiseAmount -= ctx.commonContext.smallBlind;
            }
        }
        if (ctx.commonContext.preflopCallsNumber > 0)
        { // increase raise amount if there are limpers
            myRaiseAmount += (ctx.commonContext.preflopCallsNumber * bigBlind);
        }
    }
    else
    {

        int totalPot = ctx.commonContext.sets;

        if (nbRaises == 1)
        { // will 3bet
            myRaiseAmount =
                totalPot *
                (ctx.perPlayerContext.myPosition > ctx.commonContext.preflopLastRaiser->getPosition() ? 1.2 : 1.4);
        }
        if (nbRaises > 1)
        { // will 4bet or more
            myRaiseAmount =
                totalPot *
                (ctx.perPlayerContext.myPosition > ctx.commonContext.preflopLastRaiser->getPosition() ? 1 : 1.2);
        }
    }

    // if i would be commited in the pot with the computed amount, just go allin preflop
    if (myRaiseAmount > (ctx.perPlayerContext.myCash * 0.3))
    {
        myRaiseAmount = ctx.perPlayerContext.myCash;
    }

    return myRaiseAmount;
}

bool IBotStrategy::shouldPotControl(CurrentHandContext& ctx)
{
    assert(ctx.commonContext.gameState == GameStateFlop || ctx.commonContext.gameState == GameStateTurn);

    const int bigBlind = ctx.commonContext.smallBlind * 2;
    const int potThreshold = (ctx.commonContext.gameState == GameStateFlop) ? bigBlind * 20 : bigBlind * 40;
    bool potControl = false;

    if (ctx.commonContext.pot >= potThreshold)
    {
        if (ctx.perPlayerContext.myPostFlopAnalysisFlags.isPocketPair &&
            !ctx.perPlayerContext.myPostFlopAnalysisFlags.isOverPair)
        {
            potControl = true;
        }

        if (ctx.perPlayerContext.myPostFlopAnalysisFlags.isFullHousePossible &&
            !(ctx.perPlayerContext.myPostFlopAnalysisFlags.isTrips ||
              ctx.perPlayerContext.myPostFlopAnalysisFlags.isFlush ||
              ctx.perPlayerContext.myPostFlopAnalysisFlags.isFullHouse ||
              ctx.perPlayerContext.myPostFlopAnalysisFlags.isQuads))
        {
            potControl = true;
        }

        if (ctx.commonContext.gameState == GameStateFlop)
        {
            if ((ctx.perPlayerContext.myPostFlopAnalysisFlags.isOverPair ||
                 ctx.perPlayerContext.myPostFlopAnalysisFlags.isTopPair) &&
                ctx.perPlayerContext.mySet > bigBlind * 20)
            {
                potControl = true;
            }
        }
        else if (ctx.commonContext.gameState == GameStateTurn)
        {
            if (ctx.perPlayerContext.myPostFlopAnalysisFlags.isOverPair ||
                (ctx.perPlayerContext.myPostFlopAnalysisFlags.isTwoPair &&
                 !ctx.perPlayerContext.myPostFlopAnalysisFlags.isFullHousePossible) ||
                (ctx.perPlayerContext.myPostFlopAnalysisFlags.isTrips && ctx.perPlayerContext.mySet > bigBlind * 60))
            {
                potControl = true;
            }
        }
    }

    if (potControl)
    {
        GlobalServices::instance().logger()->verbose("\t\tShould control pot");
    }

    return potControl;
}

} // namespace pkt::core::player