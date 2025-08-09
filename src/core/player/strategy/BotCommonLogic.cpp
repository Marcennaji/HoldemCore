#include "BotCommonLogic.h"

#include <core/engine/model/Ranges.h>
#include <core/player/Helpers.h>
#include <core/services/GlobalServices.h>
#include "CurrentHandContext.h"

using namespace std;

namespace pkt::core::player
{

bool BotCommonLogic::shouldPotControl(const CurrentHandContext& ctx)
{
    assert(ctx.commonContext.gameState == Flop || ctx.commonContext.gameState == Turn);

    const int bigBlind = ctx.commonContext.smallBlind * 2;
    const int potThreshold = (ctx.commonContext.gameState == Flop) ? bigBlind * 20 : bigBlind * 40;

    if (ctx.commonContext.pot < potThreshold)
    {
        return false; // No need for pot control if the pot is below the threshold
    }

    if (shouldPotControlForPocketPair(ctx))
    {
        logPotControl();
        return true;
    }

    if (shouldPotControlForFullHousePossibility(ctx))
    {
        logPotControl();
        return true;
    }

    if (ctx.commonContext.gameState == Flop && shouldPotControlOnFlop(ctx, bigBlind))
    {
        logPotControl();
        return true;
    }

    if (ctx.commonContext.gameState == Turn && shouldPotControlOnTurn(ctx, bigBlind))
    {
        logPotControl();
        return true;
    }

    return false;
}

bool BotCommonLogic::shouldPotControlForPocketPair(const CurrentHandContext& ctx) const
{
    return ctx.perPlayerContext.myPostFlopAnalysisFlags.isPocketPair &&
           !ctx.perPlayerContext.myPostFlopAnalysisFlags.isOverPair;
}

bool BotCommonLogic::shouldPotControlForFullHousePossibility(const CurrentHandContext& ctx) const
{
    const auto& flags = ctx.perPlayerContext.myPostFlopAnalysisFlags;
    return flags.isFullHousePossible && !(flags.isTrips || flags.isFlush || flags.isFullHouse || flags.isQuads);
}

bool BotCommonLogic::shouldPotControlOnFlop(const CurrentHandContext& ctx, int bigBlind) const
{
    const auto& flags = ctx.perPlayerContext.myPostFlopAnalysisFlags;
    return (flags.isOverPair || flags.isTopPair) && ctx.perPlayerContext.mySet > bigBlind * 20;
}

bool BotCommonLogic::shouldPotControlOnTurn(const CurrentHandContext& ctx, int bigBlind) const
{
    const auto& flags = ctx.perPlayerContext.myPostFlopAnalysisFlags;
    return flags.isOverPair || (flags.isTwoPair && !flags.isFullHousePossible) ||
           (flags.isTrips && ctx.perPlayerContext.mySet > bigBlind * 60);
}

void BotCommonLogic::logPotControl() const
{
    GlobalServices::instance().logger()->verbose("\t\tShould control pot");
}

int BotCommonLogic::computePreflopRaiseAmount(const CurrentHandContext& ctx)
{
    const int bigBlind = ctx.commonContext.smallBlind * 2;

    if (ctx.commonContext.preflopRaisesNumber == 0)
    {
        return computeFirstRaiseAmount(ctx, bigBlind);
    }

    return computeReRaiseAmount(ctx, bigBlind);
}

int BotCommonLogic::computeFirstRaiseAmount(const CurrentHandContext& ctx, int bigBlind) const
{
    int raiseAmount = (ctx.perPlayerContext.myM > 8 ? 2 * bigBlind : 1.5 * bigBlind);

    adjustRaiseForPosition(ctx, raiseAmount, bigBlind);
    adjustRaiseForLimpers(ctx, raiseAmount, bigBlind);

    return finalizeRaiseAmount(ctx, raiseAmount);
}

void BotCommonLogic::adjustRaiseForPosition(const CurrentHandContext& ctx, int& raiseAmount, int bigBlind) const
{
    if (ctx.commonContext.nbPlayers > 4)
    {
        if (ctx.perPlayerContext.myPosition < MIDDLE)
        {
            raiseAmount += bigBlind;
        }
        else if (ctx.perPlayerContext.myPosition == BUTTON)
        {
            raiseAmount -= ctx.commonContext.smallBlind;
        }
    }
}

void BotCommonLogic::adjustRaiseForLimpers(const CurrentHandContext& ctx, int& raiseAmount, int bigBlind) const
{
    if (ctx.commonContext.preflopCallsNumber > 0)
    {
        raiseAmount += (ctx.commonContext.preflopCallsNumber * bigBlind);
    }
}

int BotCommonLogic::computeReRaiseAmount(const CurrentHandContext& ctx, int bigBlind) const
{
    const int totalPot = ctx.commonContext.sets;
    const int nbRaises = ctx.commonContext.preflopRaisesNumber;

    int raiseAmount = 0;

    if (nbRaises == 1)
    {
        raiseAmount = computeThreeBetAmount(ctx, totalPot);
    }
    else if (nbRaises > 1)
    {
        raiseAmount = computeFourBetOrMoreAmount(ctx, totalPot);
    }

    return finalizeRaiseAmount(ctx, raiseAmount);
}

int BotCommonLogic::computeThreeBetAmount(const CurrentHandContext& ctx, int totalPot) const
{
    return totalPot *
           (ctx.perPlayerContext.myPosition > ctx.commonContext.preflopLastRaiser->getPosition() ? 1.2 : 1.4);
}

int BotCommonLogic::computeFourBetOrMoreAmount(const CurrentHandContext& ctx, int totalPot) const
{
    return totalPot * (ctx.perPlayerContext.myPosition > ctx.commonContext.preflopLastRaiser->getPosition() ? 1 : 1.2);
}

int BotCommonLogic::finalizeRaiseAmount(const CurrentHandContext& ctx, int raiseAmount) const
{
    if (raiseAmount > (ctx.perPlayerContext.myCash * 0.3))
    {
        return ctx.perPlayerContext.myCash; // Go all-in if committed
    }

    return raiseAmount;
}
} // namespace pkt::core::player