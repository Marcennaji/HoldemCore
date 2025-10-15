#include "BotStrategyBase.h"

#include <core/engine/actions/ActionValidator.h>
#include <core/engine/model/Ranges.h>
#include <core/engine/utils/Helpers.h>
#include <core/player/Helpers.h>
#include "CurrentHandContext.h"
#include "PokerMath.h"
#include "adapters/infrastructure/randomizer/DefaultRandomizer.h"
#include "core/player/Player.h"

using namespace std;

namespace pkt::core::player
{

BotStrategyBase::BotStrategyBase(Logger& logger, Randomizer& randomizer) : m_logger(logger), m_randomizer(&randomizer)
{
    m_preflopRangeCalculator = std::make_unique<PreflopRangeCalculator>(m_logger, *m_randomizer);
}

PlayerAction BotStrategyBase::decidePreflop(const CurrentHandContext& ctx)
{
    m_logger.decisionMaking("\n=== PREFLOP DECISION START ===");
    m_logger.decisionMaking("Strategy: " + getName() + ", Player: " + std::to_string(ctx.personalContext.id) +
                            ", Hand: " + ctx.personalContext.holeCards.toString());
    m_logger.decisionMaking("Position: " + std::to_string(static_cast<int>(ctx.personalContext.position)) +
                            ", Cash: " + std::to_string(ctx.personalContext.cash) +
                            ", Pot: " + std::to_string(ctx.commonContext.bettingContext.pot));
    m_logger.decisionMaking("Preflop raises: " + std::to_string(ctx.commonContext.bettingContext.preflopRaisesNumber) +
                            ", Preflop calls: " + std::to_string(ctx.commonContext.bettingContext.preflopCallsNumber));

    PlayerAction resultingAction;
    resultingAction.playerId = ctx.personalContext.id;

    // Strategy methods now handle cash constraints internally
    bool couldCall = preflopCouldCall(ctx);
    m_logger.decisionMaking("After preflopCouldCall: " + std::string(couldCall ? "YES" : "NO"));

    int raiseAmount = preflopCouldRaise(ctx);
    m_logger.decisionMaking("After preflopCouldRaise: " + std::to_string(raiseAmount));

    // Check if we want to raise
    if (raiseAmount > 0 && canAffordToRaise(ctx, raiseAmount))
    {
        // Additional check: Is the raise amount actually higher than the current highest bet?
        // This is crucial when facing an all-in from a player with more chips than us
        const int currentHighestBet = ctx.commonContext.bettingContext.highestBetAmount;

        if (raiseAmount > currentHighestBet)
        {
            // Valid raise - our raise is higher than current bet
            resultingAction.type = ActionType::Raise;
            resultingAction.amount = raiseAmount;
            m_logger.decisionMaking(">>> FINAL DECISION: RAISE " + std::to_string(raiseAmount));
        }
        else if (raiseAmount >= ctx.personalContext.cash * 0.9)
        {
            // We want to commit most/all of our stack, but it's not enough to raise
            // Convert to ALL-IN or CALL depending on whether we have enough to call
            const int callAmount = getCallAmount(ctx);

            if (ctx.personalContext.cash >= callAmount)
            {
                // We have enough to call the all-in
                resultingAction.type = ActionType::Call;
                m_logger.decisionMaking(">>> FINAL DECISION: CALL (wanted to raise " + std::to_string(raiseAmount) +
                                        " but not enough to raise above " + std::to_string(currentHighestBet) + ")");
            }
            else
            {
                // We don't have enough to call - go all-in with what we have
                resultingAction.type = ActionType::Allin;
                resultingAction.amount = ctx.personalContext.cash;
                m_logger.decisionMaking(">>> FINAL DECISION: ALL-IN " + std::to_string(ctx.personalContext.cash) +
                                        " (wanted to raise but not enough chips)");
            }
        }
        else
        {
            // We computed a raise but it's not high enough and we're not fully committed
            // Fall back to call/fold logic
            if (couldCall && canAffordToCall(ctx))
            {
                resultingAction.type = ActionType::Call;
                m_logger.decisionMaking(">>> FINAL DECISION: CALL (raise amount " + std::to_string(raiseAmount) +
                                        " not sufficient to raise above " + std::to_string(currentHighestBet) + ")");
            }
            else
            {
                resultingAction.type = ActionType::Fold;
                m_logger.decisionMaking(">>> FINAL DECISION: FOLD (raise amount insufficient and cannot/won't call)");
            }
        }
    }
    else if (couldCall)
    {
        resultingAction.type = ActionType::Call;
        m_logger.decisionMaking(">>> FINAL DECISION: CALL");
    }
    else if (ctx.commonContext.bettingContext.preflopRaisesNumber == 0 &&
             ctx.personalContext.position == PlayerPosition::BigBlind)
    {
        // If last to speak, hand not good enough to raise/call, and nobody has raised: check
        resultingAction.type = ActionType::Check;
        m_logger.decisionMaking(">>> FINAL DECISION: CHECK (BB with no raises)");
    }
    else
    {
        resultingAction.type = ActionType::Fold;
        m_logger.decisionMaking(">>> FINAL DECISION: FOLD");
    }

    m_logger.decisionMaking("=== PREFLOP DECISION END ===\n");

    return resultingAction;
}
PlayerAction BotStrategyBase::decideFlop(const CurrentHandContext& ctx)
{
    m_logger.decisionMaking("\n=== FLOP DECISION START ===");
    m_logger.decisionMaking("Strategy: " + getName() + ", Player: " + std::to_string(ctx.personalContext.id) +
                            ", Hand: " + ctx.personalContext.holeCards.toString());
    m_logger.decisionMaking("Flop bets/raises: " +
                            std::to_string(ctx.commonContext.bettingContext.flopBetsOrRaisesNumber));

    PlayerAction resultingAction;
    resultingAction.playerId = ctx.personalContext.id;

    if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber == 0)
    {
        // No bets yet - decide whether to bet or check
        m_logger.decisionMaking("No bets yet - checking if should bet");
        int betAmount = flopCouldBet(ctx);
        if (betAmount > 0 && betAmount <= ctx.personalContext.cash)
        {
            resultingAction.type = ActionType::Bet;
            resultingAction.amount = betAmount;
            m_logger.decisionMaking(">>> FINAL DECISION: BET " + std::to_string(betAmount));
        }
        else
        {
            resultingAction.type = ActionType::Check;
            m_logger.decisionMaking(">>> FINAL DECISION: CHECK");
        }
    }
    else
    {
        // There are bets/raises - decide whether to call, raise, or fold
        m_logger.decisionMaking("There are bets/raises - deciding action");
        int raiseAmount = flopCouldRaise(ctx);
        if (raiseAmount > 0 && canAffordToRaise(ctx, raiseAmount))
        {
            // Check if raise is valid (higher than current bet)
            const int currentHighestBet = ctx.commonContext.bettingContext.highestBetAmount;

            if (raiseAmount > currentHighestBet)
            {
                resultingAction.type = ActionType::Raise;
                resultingAction.amount = raiseAmount;
                m_logger.decisionMaking(">>> FINAL DECISION: RAISE " + std::to_string(raiseAmount));
            }
            else if (raiseAmount >= ctx.personalContext.cash * 0.9 && flopCouldCall(ctx))
            {
                // Want to commit but not enough to raise - call instead
                resultingAction.type = ActionType::Call;
                m_logger.decisionMaking(">>> FINAL DECISION: CALL (wanted to raise but insufficient)");
            }
            else if (flopCouldCall(ctx))
            {
                resultingAction.type = ActionType::Call;
                m_logger.decisionMaking(">>> FINAL DECISION: CALL");
            }
            else
            {
                resultingAction.type = ActionType::Fold;
                m_logger.decisionMaking(">>> FINAL DECISION: FOLD");
            }
        }
        else if (flopCouldCall(ctx))
        {
            resultingAction.type = ActionType::Call;
            m_logger.decisionMaking(">>> FINAL DECISION: CALL");
        }
        else
        {
            resultingAction.type = ActionType::Fold;
            m_logger.decisionMaking(">>> FINAL DECISION: FOLD");
        }
    }

    m_logger.decisionMaking("=== FLOP DECISION END ===\n");

    return resultingAction;
}
PlayerAction BotStrategyBase::decideTurn(const CurrentHandContext& ctx)
{
    PlayerAction resultingAction;
    resultingAction.playerId = ctx.personalContext.id;

    if (ctx.commonContext.bettingContext.turnBetsOrRaisesNumber == 0)
    {
        // No bets yet - decide whether to bet or check
        int betAmount = turnCouldBet(ctx);
        if (betAmount > 0 && betAmount <= ctx.personalContext.cash)
        {
            resultingAction.type = ActionType::Bet;
            resultingAction.amount = betAmount;
        }
        else
        {
            resultingAction.type = ActionType::Check;
        }
    }
    else
    {
        // There are bets/raises - decide whether to call, raise, or fold
        int raiseAmount = turnCouldRaise(ctx);
        if (raiseAmount > 0 && canAffordToRaise(ctx, raiseAmount))
        {
            // Check if raise is valid (higher than current bet)
            const int currentHighestBet = ctx.commonContext.bettingContext.highestBetAmount;

            if (raiseAmount > currentHighestBet)
            {
                resultingAction.type = ActionType::Raise;
                resultingAction.amount = raiseAmount;
            }
            else if (raiseAmount >= ctx.personalContext.cash * 0.9 && turnCouldCall(ctx))
            {
                // Want to commit but not enough to raise - call instead
                resultingAction.type = ActionType::Call;
            }
            else if (turnCouldCall(ctx))
            {
                resultingAction.type = ActionType::Call;
            }
            else
            {
                resultingAction.type = ActionType::Fold;
            }
        }
        else if (turnCouldCall(ctx))
        {
            resultingAction.type = ActionType::Call;
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

    if (ctx.commonContext.bettingContext.riverBetsOrRaisesNumber == 0)
    {
        // No bets yet - decide whether to bet or check
        int betAmount = riverCouldBet(ctx);
        if (betAmount > 0 && betAmount <= ctx.personalContext.cash)
        {
            resultingAction.type = ActionType::Bet;
            resultingAction.amount = betAmount;
        }
        else
        {
            resultingAction.type = ActionType::Check;
        }
    }
    else
    {
        // There are bets/raises - decide whether to call, raise, or fold
        int raiseAmount = riverCouldRaise(ctx);
        if (raiseAmount > 0 && canAffordToRaise(ctx, raiseAmount))
        {
            // Check if raise is valid (higher than current bet)
            const int currentHighestBet = ctx.commonContext.bettingContext.highestBetAmount;

            if (raiseAmount > currentHighestBet)
            {
                resultingAction.type = ActionType::Raise;
                resultingAction.amount = raiseAmount;
            }
            else if (raiseAmount >= ctx.personalContext.cash * 0.9 && riverCouldCall(ctx))
            {
                // Want to commit but not enough to raise - call instead
                resultingAction.type = ActionType::Call;
            }
            else if (riverCouldCall(ctx))
            {
                resultingAction.type = ActionType::Call;
            }
            else
            {
                resultingAction.type = ActionType::Fold;
            }
        }
        else if (riverCouldCall(ctx))
        {
            resultingAction.type = ActionType::Call;
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
    m_logger.verbose("\t\tShould control pot");
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
    // Phase 3: Use PokerMath utilities for cleaner, centralized logic
    float baseRaise = PokerMath::calculateStandardOpenRaise(ctx);
    float positionAdjustment = PokerMath::getPositionRaiseAdjustment(ctx);
    float limperAdjustment = PokerMath::getLimperRaiseAdjustment(ctx);

    int raiseAmount = static_cast<int>(baseRaise + positionAdjustment + limperAdjustment);

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
    // Phase 3: Use PokerMath utilities for centralized 3-bet sizing
    assert(ctx.commonContext.playersContext.preflopLastRaiser != nullptr);
    return static_cast<int>(PokerMath::calculate3BetSize(ctx));
}

int BotStrategyBase::computeFourBetOrMoreAmount(const CurrentHandContext& ctx, int totalPot) const
{
    // Phase 3: Use PokerMath utilities for centralized 4-bet+ sizing
    assert(ctx.commonContext.playersContext.preflopLastRaiser != nullptr);
    return static_cast<int>(PokerMath::calculate4BetPlusSize(ctx));
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

    PlayerList players = ctx.commonContext.playersContext.actingPlayersList;

    if (players == nullptr)
    {
        m_logger.info("BotStrategyBase::isPossibleToBluff() is not compatible with legacy (non FSM) code");
        return false; // TODO remove this after FSM migration is complete
    }

    if (players->size() == 1)
    {
        // all other players are allin
        return false;
    }

    for (PlayerListIterator it = players->begin(); it != players->end(); ++it)
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

bool BotStrategyBase::canAffordToCall(const CurrentHandContext& ctx) const
{
    const int callAmount = getCallAmount(ctx);
    return ctx.personalContext.cash >= callAmount;
}

bool BotStrategyBase::canAffordToRaise(const CurrentHandContext& ctx, int raiseAmount) const
{
    const int currentPlayerBet =
        ctx.personalContext.actions.currentHandActions.getRoundTotalBetAmount(ctx.commonContext.gameState);
    const int extraCashRequired = raiseAmount - currentPlayerBet;
    return ctx.personalContext.cash >= extraCashRequired;
}

int BotStrategyBase::getCallAmount(const CurrentHandContext& ctx) const
{
    const int currentPlayerBet =
        ctx.personalContext.actions.currentHandActions.getRoundTotalBetAmount(ctx.commonContext.gameState);
    return ctx.commonContext.bettingContext.highestBetAmount - currentPlayerBet;
}
} // namespace pkt::core::player