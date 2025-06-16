#include "IBotStrategy.h"
#include <core/engine/Randomizer.h>
#include <core/engine/model/Ranges.h>
#include <core/player/Helpers.h>
#include "CurrentHandContext.h"

using namespace std;

namespace pkt::core::player
{

void IBotStrategy::initializeRanges(const int utgHeadsUpRange, const int utgFullTableRange)
{
    myPreflopRangeCalculator->initializeRanges(utgHeadsUpRange, utgFullTableRange);
}

int IBotStrategy::computePreflopRaiseAmount(CurrentHandContext& ctx, bool deterministic)
{
    int myRaiseAmount = 0;

    const int nbRaises = ctx.preflopRaisesNumber;
    const int nbCalls = ctx.preflopCallsNumber;
    const int nbPlayers = ctx.nbPlayers;

    const int bigBlind = ctx.smallBlind * 2;

    if (nbRaises == 0)
    { // first to raise

        myRaiseAmount = (ctx.myM > 8 ? 2 * bigBlind : 1.5 * bigBlind);

        if (nbPlayers > 4)
        { // adjust for position
            if (ctx.myPosition < MIDDLE)
                myRaiseAmount += bigBlind;
            if (ctx.myPosition == BUTTON)
                myRaiseAmount -= ctx.smallBlind;
        }
        if (ctx.preflopCallsNumber > 0) // increase raise amount if there are limpers
            myRaiseAmount += (ctx.preflopCallsNumber * bigBlind);
    }
    else
    {

        int totalPot = ctx.sets;

        if (nbRaises == 1)
        { // will 3bet
            myRaiseAmount = totalPot * (ctx.myPosition > ctx.preflopLastRaiser->getPosition() ? 1.2 : 1.4);
        }
        if (nbRaises > 1)
        { // will 4bet or more
            myRaiseAmount = totalPot * (ctx.myPosition > ctx.preflopLastRaiser->getPosition() ? 1 : 1.2);
        }
    }

    // if i would be commited in the pot with the computed amount, just go allin preflop
    if (myRaiseAmount > (ctx.myCash * 0.3))
        myRaiseAmount = ctx.myCash;

    return myRaiseAmount;
}

bool IBotStrategy::shouldPotControl(CurrentHandContext& ctx, bool deterministic)
{

    assert(ctx.gameState == GAME_STATE_FLOP || ctx.gameState == GAME_STATE_TURN);

    bool potControl = false;
    const int bigBlind = ctx.smallBlind * 2;

    if (ctx.gameState == GAME_STATE_FLOP &&
        !(ctx.preflopRaisesNumber > 0 && ctx.preflopLastRaiser->getID() == ctx.myID && ctx.flopBetsOrRaisesNumber == 0))
    {

        if (ctx.pot >= bigBlind * 20)
        {

            if (ctx.myPostFlopState.IsPocketPair && !ctx.myPostFlopState.IsOverPair)
                potControl = true;

            if (ctx.myPostFlopState.IsFullHousePossible &&
                !(ctx.myPostFlopState.IsTrips || ctx.myPostFlopState.IsFlush || ctx.myPostFlopState.IsFullHouse ||
                  ctx.myPostFlopState.IsQuads))
                potControl = true;

            if ((ctx.myPostFlopState.IsOverPair || ctx.myPostFlopState.IsTopPair) && ctx.mySet > bigBlind * 20)
                potControl = true;
        }
    }
    else

        if (ctx.gameState == GAME_STATE_TURN)
    {

        if (ctx.pot >= bigBlind * 40)
        {

            if (ctx.myPostFlopState.IsPocketPair && !ctx.myPostFlopState.IsOverPair)
                potControl = true;

            if (ctx.myPostFlopState.IsOverPair)
                potControl = true;

            if (ctx.myPostFlopState.IsFullHousePossible &&
                !(ctx.myPostFlopState.IsTrips || ctx.myPostFlopState.IsFlush || ctx.myPostFlopState.IsFullHouse ||
                  ctx.myPostFlopState.IsQuads))
                potControl = true;

            // 2 pairs
            if (ctx.myPostFlopState.IsTwoPair && !ctx.myPostFlopState.IsFullHousePossible)
                potControl = true;

            if (ctx.myPostFlopState.IsTrips && ctx.mySet > bigBlind * 60)
                potControl = true;
        }
    }

    if (potControl)
        myLogger->info("\t\tShould control pot");

    return potControl;
}

} // namespace pkt::core::player