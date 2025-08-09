#include "BotStrategyBase.h"

#include <core/engine/model/Ranges.h>
#include <core/player/Helpers.h>
#include <core/services/GlobalServices.h>
#include "CurrentHandContext.h"

using namespace std;

namespace pkt::core::player
{
PlayerAction BotStrategyBase::decidePreflop(const CurrentHandContext& ctx)
{
    PlayerAction resultingAction;

    bool shouldCall = preflopShouldCall(ctx);         // should at least call, and maybe raise
    resultingAction.amount = preflopShouldRaise(ctx); // amount > 0 if decide to raise

    if (resultingAction.amount > 0)
    {
        shouldCall = false;
    }

    // if last to speak, a hand not good enough to raise, and nobody has raised : I check
    if (ctx.commonContext.preflopRaisesNumber == 0 && resultingAction.amount == 0 &&
        ctx.perPlayerContext.myPosition == BB)
    {
        resultingAction.type = ActionType::Check;
    }
    else
    {
        if (shouldCall)
        {
            resultingAction.type = ActionType::Call;
        }
        else if (resultingAction.amount > 0)
        {
            resultingAction.type = ActionType::Raise;
        }
        else
        {
            resultingAction.type = ActionType::Fold;
        }
    }
    return resultingAction;
}
PlayerAction BotStrategyBase::decideFlop(const CurrentHandContext& ctx)
{
    PlayerAction resultingAction;
    int betAmount = 0;
    int raiseAmount = 0;
    bool shouldCall = false;

    if (ctx.commonContext.flopBetsOrRaisesNumber == 0)
    {
        betAmount = flopShouldBet(ctx);
    }
    else
    {
        shouldCall = flopShouldCall(ctx);
        raiseAmount = flopShouldRaise(ctx);
    }

    if (raiseAmount)
    {
        shouldCall = false;
    }

    if (ctx.commonContext.flopBetsOrRaisesNumber == 0 && !raiseAmount && !betAmount)
    {
        resultingAction.type = ActionType::Check;
    }
    else
    {
        if (betAmount)
        {
            resultingAction.type = ActionType::Bet;
            resultingAction.amount = betAmount;
        }
        else if (shouldCall)
        {
            resultingAction.type = ActionType::Call;
        }
        else if (raiseAmount)
        {
            resultingAction.type = ActionType::Raise;
            resultingAction.amount = raiseAmount;
        }
        else
        {
            resultingAction.type = ActionType::Fold;
        }
    }

    return resultingAction;
}
PlayerAction BotStrategyBase::decideTurn(const CurrentHandContext& ctx)
{
    PlayerAction resultingAction;
    int betAmount = 0;
    int raiseAmount = 0;

    bool shouldCall = false;

    if (ctx.commonContext.turnBetsOrRaisesNumber == 0)
    {
        betAmount = turnShouldBet(ctx);
    }
    else
    {
        shouldCall = turnShouldCall(ctx);
        raiseAmount = turnShouldRaise(ctx);
    }

    if (raiseAmount)
    {
        shouldCall = false;
    }

    if (ctx.commonContext.turnBetsOrRaisesNumber == 0 && !raiseAmount && !betAmount)
    {
        resultingAction.type = ActionType::Check;
    }
    else
    {
        if (betAmount)
        {
            resultingAction.type = ActionType::Bet;
            resultingAction.amount = betAmount;
        }
        else if (shouldCall)
        {
            resultingAction.type = ActionType::Call;
        }
        else if (raiseAmount)
        {
            resultingAction.type = ActionType::Raise;
            resultingAction.amount = raiseAmount;
        }
        else
        {
            resultingAction.type = ActionType::Fold;
        }
    }
    return resultingAction;
}
PlayerAction BotStrategyBase::decideRiver(const CurrentHandContext& ctx)
{
    PlayerAction resultingAction;
    int betAmount = 0;
    int raiseAmount = 0;
    bool shouldCall = false;

    if (ctx.commonContext.riverBetsOrRaisesNumber == 0)
    {
        betAmount = riverShouldBet(ctx);
    }
    else
    {
        shouldCall = riverShouldCall(ctx);
        raiseAmount = riverShouldRaise(ctx);
    }

    if (raiseAmount)
    {
        shouldCall = false;
    }

    if (ctx.commonContext.riverBetsOrRaisesNumber == 0 && !raiseAmount && !betAmount)
    {
        resultingAction.type = ActionType::Check;
    }
    else
    {
        if (betAmount)
        {
            resultingAction.type = ActionType::Bet;
            resultingAction.amount = betAmount;
        }
        else if (shouldCall)
        {
            resultingAction.type = ActionType::Call;
        }
        else if (raiseAmount)
        {
            resultingAction.type = ActionType::Raise;
            resultingAction.amount = raiseAmount;
        }
        else
        {
            resultingAction.type = ActionType::Fold;
        }
    }
    return resultingAction;
}
bool BotStrategyBase::shouldPotControl(const CurrentHandContext& ctx)
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

bool BotStrategyBase::shouldPotControlForPocketPair(const CurrentHandContext& ctx) const
{
    return ctx.perPlayerContext.myPostFlopAnalysisFlags.isPocketPair &&
           !ctx.perPlayerContext.myPostFlopAnalysisFlags.isOverPair;
}

bool BotStrategyBase::shouldPotControlForFullHousePossibility(const CurrentHandContext& ctx) const
{
    const auto& flags = ctx.perPlayerContext.myPostFlopAnalysisFlags;
    return flags.isFullHousePossible && !(flags.isTrips || flags.isFlush || flags.isFullHouse || flags.isQuads);
}

bool BotStrategyBase::shouldPotControlOnFlop(const CurrentHandContext& ctx, int bigBlind) const
{
    const auto& flags = ctx.perPlayerContext.myPostFlopAnalysisFlags;
    return (flags.isOverPair || flags.isTopPair) && ctx.perPlayerContext.mySet > bigBlind * 20;
}

bool BotStrategyBase::shouldPotControlOnTurn(const CurrentHandContext& ctx, int bigBlind) const
{
    const auto& flags = ctx.perPlayerContext.myPostFlopAnalysisFlags;
    return flags.isOverPair || (flags.isTwoPair && !flags.isFullHousePossible) ||
           (flags.isTrips && ctx.perPlayerContext.mySet > bigBlind * 60);
}

void BotStrategyBase::logPotControl() const
{
    GlobalServices::instance().logger()->verbose("\t\tShould control pot");
}

int BotStrategyBase::computePreflopRaiseAmount(const CurrentHandContext& ctx)
{
    const int bigBlind = ctx.commonContext.smallBlind * 2;

    if (ctx.commonContext.preflopRaisesNumber == 0)
    {
        return computeFirstRaiseAmount(ctx, bigBlind);
    }

    return computeReRaiseAmount(ctx, bigBlind);
}

int BotStrategyBase::computeFirstRaiseAmount(const CurrentHandContext& ctx, int bigBlind) const
{
    int raiseAmount = (ctx.perPlayerContext.myM > 8 ? 2 * bigBlind : 1.5 * bigBlind);

    adjustRaiseForPosition(ctx, raiseAmount, bigBlind);
    adjustRaiseForLimpers(ctx, raiseAmount, bigBlind);

    return finalizeRaiseAmount(ctx, raiseAmount);
}

void BotStrategyBase::adjustRaiseForPosition(const CurrentHandContext& ctx, int& raiseAmount, int bigBlind) const
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

void BotStrategyBase::adjustRaiseForLimpers(const CurrentHandContext& ctx, int& raiseAmount, int bigBlind) const
{
    if (ctx.commonContext.preflopCallsNumber > 0)
    {
        raiseAmount += (ctx.commonContext.preflopCallsNumber * bigBlind);
    }
}

int BotStrategyBase::computeReRaiseAmount(const CurrentHandContext& ctx, int bigBlind) const
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

int BotStrategyBase::computeThreeBetAmount(const CurrentHandContext& ctx, int totalPot) const
{
    return totalPot *
           (ctx.perPlayerContext.myPosition > ctx.commonContext.preflopLastRaiser->getPosition() ? 1.2 : 1.4);
}

int BotStrategyBase::computeFourBetOrMoreAmount(const CurrentHandContext& ctx, int totalPot) const
{
    return totalPot * (ctx.perPlayerContext.myPosition > ctx.commonContext.preflopLastRaiser->getPosition() ? 1 : 1.2);
}

int BotStrategyBase::finalizeRaiseAmount(const CurrentHandContext& ctx, int raiseAmount) const
{
    if (raiseAmount > (ctx.perPlayerContext.myCash * 0.3))
    {
        return ctx.perPlayerContext.myCash; // Go all-in if committed
    }

    return raiseAmount;
}
} // namespace pkt::core::player