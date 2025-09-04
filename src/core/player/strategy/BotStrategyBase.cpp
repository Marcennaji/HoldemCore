#include "BotStrategyBase.h"

#include <core/engine/model/Ranges.h>
#include <core/player/Helpers.h>
#include <core/services/GlobalServices.h>
#include "CurrentHandContext.h"
#include "core/player/PlayerFsm.h"

using namespace std;

namespace pkt::core::player
{
PlayerAction BotStrategyBase::decidePreflop(const CurrentHandContext& ctx)
{
    PlayerAction resultingAction;
    resultingAction.playerId = ctx.personalContext.id;

    bool shouldCall = preflopShouldCall(ctx);         // should at least call, and maybe raise
    resultingAction.amount = preflopShouldRaise(ctx); // amount > 0 if decide to raise

    if (resultingAction.amount > 0)
    {
        shouldCall = false;
    }

    // if last to speak, a hand not good enough to raise, and nobody has raised : I check
    if (ctx.commonContext.bettingContext.preflopRaisesNumber == 0 && resultingAction.amount == 0 &&
        ctx.personalContext.position == PlayerPosition::BigBlind)
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
    resultingAction.playerId = ctx.personalContext.id;

    int betAmount = 0;
    int raiseAmount = 0;
    bool shouldCall = false;

    if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber == 0)
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

    if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber == 0 && !raiseAmount && !betAmount)
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
    resultingAction.playerId = ctx.personalContext.id;

    int betAmount = 0;
    int raiseAmount = 0;

    bool shouldCall = false;

    if (ctx.commonContext.bettingContext.turnBetsOrRaisesNumber == 0)
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

    if (ctx.commonContext.bettingContext.turnBetsOrRaisesNumber == 0 && !raiseAmount && !betAmount)
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
    resultingAction.playerId = ctx.personalContext.id;

    int betAmount = 0;
    int raiseAmount = 0;
    bool shouldCall = false;

    if (ctx.commonContext.bettingContext.riverBetsOrRaisesNumber == 0)
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

    if (ctx.commonContext.bettingContext.riverBetsOrRaisesNumber == 0 && !raiseAmount && !betAmount)
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

    if (ctx.commonContext.bettingContext.pot < potThreshold)
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
    return ctx.personalContext.postFlopAnalysisFlags.isPocketPair &&
           !ctx.personalContext.postFlopAnalysisFlags.isOverPair;
}

bool BotStrategyBase::shouldPotControlForFullHousePossibility(const CurrentHandContext& ctx) const
{
    const auto& flags = ctx.personalContext.postFlopAnalysisFlags;
    return flags.isFullHousePossible && !(flags.isTrips || flags.isFlush || flags.isFullHouse || flags.isQuads);
}

bool BotStrategyBase::shouldPotControlOnFlop(const CurrentHandContext& ctx, int bigBlind) const
{
    const auto& flags = ctx.personalContext.postFlopAnalysisFlags;
    return (flags.isOverPair || flags.isTopPair) && ctx.personalContext.totalBetAmount > bigBlind * 20;
}

bool BotStrategyBase::shouldPotControlOnTurn(const CurrentHandContext& ctx, int bigBlind) const
{
    const auto& flags = ctx.personalContext.postFlopAnalysisFlags;
    return flags.isOverPair || (flags.isTwoPair && !flags.isFullHousePossible) ||
           (flags.isTrips && ctx.personalContext.totalBetAmount > bigBlind * 60);
}

void BotStrategyBase::logPotControl() const
{
    GlobalServices::instance().logger().verbose("\t\tShould control pot");
}

int BotStrategyBase::computePreflopRaiseAmount(const CurrentHandContext& ctx)
{
    const int bigBlind = ctx.commonContext.smallBlind * 2;

    if (ctx.commonContext.bettingContext.preflopRaisesNumber == 0)
    {
        return computeFirstRaiseAmount(ctx, bigBlind);
    }

    return computeReRaiseAmount(ctx, bigBlind);
}

int BotStrategyBase::computeFirstRaiseAmount(const CurrentHandContext& ctx, int bigBlind) const
{
    int raiseAmount = (ctx.personalContext.m > 8 ? 2 * bigBlind : 1.5 * bigBlind);

    adjustRaiseForPosition(ctx, raiseAmount, bigBlind);
    adjustRaiseForLimpers(ctx, raiseAmount, bigBlind);

    return finalizeRaiseAmount(ctx, raiseAmount);
}

void BotStrategyBase::adjustRaiseForPosition(const CurrentHandContext& ctx, int& raiseAmount, int bigBlind) const
{
    if (ctx.commonContext.playersContext.nbPlayers > 4)
    {
        if (ctx.personalContext.position < Middle)
        {
            raiseAmount += bigBlind;
        }
        else if (ctx.personalContext.position == Button)
        {
            raiseAmount -= ctx.commonContext.smallBlind;
        }
    }
}

void BotStrategyBase::adjustRaiseForLimpers(const CurrentHandContext& ctx, int& raiseAmount, int bigBlind) const
{
    if (ctx.commonContext.bettingContext.preflopCallsNumber > 0)
    {
        raiseAmount += (ctx.commonContext.bettingContext.preflopCallsNumber * bigBlind);
    }
}

int BotStrategyBase::computeReRaiseAmount(const CurrentHandContext& ctx, int bigBlind) const
{
    const int totalPot = ctx.commonContext.bettingContext.sets;
    const int nbRaises = ctx.commonContext.bettingContext.preflopRaisesNumber;

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
    assert(ctx.commonContext.playersContext.preflopLastRaiser != nullptr);
    return totalPot * (ctx.personalContext.position > ctx.commonContext.playersContext.preflopLastRaiser->getPosition()
                           ? 1.2
                           : 1.4);
}

int BotStrategyBase::computeFourBetOrMoreAmount(const CurrentHandContext& ctx, int totalPot) const
{
    assert(ctx.commonContext.playersContext.preflopLastRaiser != nullptr);
    return totalPot *
           (ctx.personalContext.position > ctx.commonContext.playersContext.preflopLastRaiser->getPosition() ? 1 : 1.2);
}

int BotStrategyBase::finalizeRaiseAmount(const CurrentHandContext& ctx, int raiseAmount) const
{
    if (raiseAmount > (ctx.personalContext.cash * 0.3))
    {
        return ctx.personalContext.cash; // Go all-in if committed
    }

    return raiseAmount;
}
bool BotStrategyBase::isPossibleToBluff(const CurrentHandContext& ctx) const
{

    // check if there is no calling station at the table
    // check also if my opponents stacks are big enough to bluff them

    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    const int nbRaises = ctx.commonContext.bettingContext.preflopRaisesNumber;

    PlayerFsmList players = ctx.commonContext.playersContext.runningPlayersListFsm;

    if (players == nullptr)
    {
        GlobalServices::instance().logger().info(
            "BotStrategyBase::isPossibleToBluff() is not compatible with legacy (non FSM) code");
        return false; // TODO remove this after FSM migration is complete
    }

    if (players->size() == 1)
    {
        // all other players are allin
        return false;
    }

    for (PlayerFsmListIterator it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getId() == ctx.personalContext.id)
        {
            continue;
        }

        PreflopStatistics preflopStats = (*it)->getStatisticsUpdater()->getStatistics(nbPlayers).preflopStatistics;

        // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more
        // accurate
        if (preflopStats.hands < MIN_HANDS_STATISTICS_ACCURATE && nbPlayers < 10 &&
            (*it)->getStatisticsUpdater()->getStatistics(nbPlayers + 1).preflopStatistics.hands >
                MIN_HANDS_STATISTICS_ACCURATE)
        {

            preflopStats = (*it)->getStatisticsUpdater()->getStatistics(nbPlayers + 1).preflopStatistics;
        }

        if ((*it)->getStatisticsUpdater()->getStatistics(nbPlayers).getWentToShowDown() >= 40 &&
            preflopStats.getVoluntaryPutMoneyInPot() - preflopStats.getPreflopRaise() > 15 &&
            preflopStats.getVoluntaryPutMoneyInPot() > 20)
        {
            return false; // seems to be a calling station
        }

        if ((*it)->getCash() < ctx.commonContext.bettingContext.pot * 3)
        {
            return false;
        }

        if (ctx.commonContext.gameState == Preflop)
        {
            if (preflopStats.getPreflopCallthreeBetsFrequency() > 40)
            {
                return false;
            }
        }
    }

    return true;
}
} // namespace pkt::core::player