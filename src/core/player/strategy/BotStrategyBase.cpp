#include "BotStrategyBase.h"

#include <core/engine/model/Ranges.h>
#include <core/engine/actions/ActionValidator.h>
#include <core/engine/utils/Helpers.h>
#include <core/player/Helpers.h>
#include "CurrentHandContext.h"
#include "core/player/Player.h"
#include "PokerMath.h"  // Phase 3: Include utilities for preflop calculations

using namespace std;

namespace pkt::core::player
{

BotStrategyBase::BotStrategyBase()
{
    // Default constructor - services will be initialized lazily
}

BotStrategyBase::BotStrategyBase(std::shared_ptr<pkt::core::ServiceContainer> services) : m_services(services)
{
    // Constructor with services injection
    // Also ensure our PreflopRangeCalculator uses the same injected services
    if (m_services && m_preflopRangeCalculator)
    {
        // Replace the default-constructed calculator with one bound to injected services
        m_preflopRangeCalculator = std::make_unique<PreflopRangeCalculator>(m_services);
    }
}

void BotStrategyBase::ensureServicesInitialized() const
{
    if (!m_services)
    {
        // Use a shared default container to avoid repeated allocations and to keep a consistent default
        static std::shared_ptr<pkt::core::ServiceContainer> defaultServices =
            std::make_shared<pkt::core::AppServiceContainer>();
        m_services = defaultServices;
    }
}

PlayerAction BotStrategyBase::decidePreflop(const CurrentHandContext& ctx)
{
    ensureServicesInitialized();
    PlayerAction resultingAction;
    resultingAction.playerId = ctx.personalContext.id;

    // Strategy methods now handle cash constraints internally
    bool couldCall = preflopCouldCall(ctx);
    int raiseAmount = preflopCouldRaise(ctx);

    if (raiseAmount > 0)
    {
        resultingAction.type = ActionType::Raise;
        resultingAction.amount = raiseAmount;
    }
    else if (couldCall)
    {
        resultingAction.type = ActionType::Call;
    }
    else if (ctx.commonContext.bettingContext.preflopRaisesNumber == 0 && 
             ctx.personalContext.position == PlayerPosition::BigBlind)
    {
        // If last to speak, hand not good enough to raise/call, and nobody has raised: check
        resultingAction.type = ActionType::Check;
    }
    else
    {
        resultingAction.type = ActionType::Fold;
    }
    
    return resultingAction;
}
PlayerAction BotStrategyBase::decideFlop(const CurrentHandContext& ctx)
{
    ensureServicesInitialized();
    PlayerAction resultingAction;
    resultingAction.playerId = ctx.personalContext.id;

    if (ctx.commonContext.bettingContext.flopBetsOrRaisesNumber == 0)
    {
        // No bets yet - decide whether to bet or check
        int betAmount = flopCouldBet(ctx);
        if (betAmount > 0)
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
        int raiseAmount = flopCouldRaise(ctx);
        if (raiseAmount > 0)
        {
            resultingAction.type = ActionType::Raise;
            resultingAction.amount = raiseAmount;
        }
        else if (flopCouldCall(ctx))
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
PlayerAction BotStrategyBase::decideTurn(const CurrentHandContext& ctx)
{
    ensureServicesInitialized();
    PlayerAction resultingAction;
    resultingAction.playerId = ctx.personalContext.id;

    if (ctx.commonContext.bettingContext.turnBetsOrRaisesNumber == 0)
    {
        // No bets yet - decide whether to bet or check
        int betAmount = turnCouldBet(ctx);
        if (betAmount > 0)
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
        if (raiseAmount > 0)
        {
            resultingAction.type = ActionType::Raise;
            resultingAction.amount = raiseAmount;
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
    ensureServicesInitialized();
    PlayerAction resultingAction;
    resultingAction.playerId = ctx.personalContext.id;

    if (ctx.commonContext.bettingContext.riverBetsOrRaisesNumber == 0)
    {
        // No bets yet - decide whether to bet or check
        int betAmount = riverCouldBet(ctx);
        if (betAmount > 0)
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
        if (raiseAmount > 0)
        {
            resultingAction.type = ActionType::Raise;
            resultingAction.amount = raiseAmount;
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
    ensureServicesInitialized();
    m_services->logger().verbose("\t\tShould control pot");
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
        ensureServicesInitialized();
        m_services->logger().info("BotStrategyBase::isPossibleToBluff() is not compatible with legacy (non FSM) code");
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
    const int currentPlayerBet = ctx.personalContext.actions.currentHandActions.getRoundTotalBetAmount(ctx.commonContext.gameState);
    const int extraCashRequired = raiseAmount - currentPlayerBet;
    return ctx.personalContext.cash >= extraCashRequired;
}

int BotStrategyBase::getCallAmount(const CurrentHandContext& ctx) const
{
    const int currentPlayerBet = ctx.personalContext.actions.currentHandActions.getRoundTotalBetAmount(ctx.commonContext.gameState);
    return ctx.commonContext.bettingContext.highestBetAmount - currentPlayerBet;
}
} // namespace pkt::core::player