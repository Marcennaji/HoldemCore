// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "PokerMath.h"
#include "CurrentHandContext.h"

namespace pkt::core::player
{

// ========================================
// Equity and Odds Calculations
// ========================================

bool PokerMath::hasInsufficientEquityForCall(const CurrentHandContext& ctx, float equityMultiplier)
{
    // This pattern appears in ALL strategy classes with minor variations:
    // if (ctx.personalContext.m_handSimulation.winRanged * 100 < ctx.commonContext.bettingContext.potOdd)
    return (ctx.personalContext.m_handSimulation.winRanged * 100.0f) < 
           (ctx.commonContext.bettingContext.potOdd * equityMultiplier);
}

bool PokerMath::hasVeryStrongEquity(const CurrentHandContext& ctx, float threshold)
{
    // Common pattern: ctx.personalContext.m_handSimulation.winRanged > 0.93 && ctx.personalContext.m_handSimulation.win > 0.X
    return ctx.personalContext.m_handSimulation.winRanged > threshold && 
           ctx.personalContext.m_handSimulation.win > 0.5f;
}

bool PokerMath::hasStrongEquityVsField(const CurrentHandContext& ctx, float threshold)
{
    return ctx.personalContext.m_handSimulation.winRanged > threshold;
}

bool PokerMath::hasWeakEquity(const CurrentHandContext& ctx, float threshold)
{
    // Common pattern: ctx.personalContext.m_handSimulation.winRanged < 0.25 && ctx.personalContext.m_handSimulation.win < 0.X
    return ctx.personalContext.m_handSimulation.winRanged < threshold && 
           ctx.personalContext.m_handSimulation.win < 0.9f;
}

// ========================================
// Stack Protection Logic
// ========================================

bool PokerMath::wouldRiskTooMuchOfStack(const CurrentHandContext& ctx)
{
    // Common pattern found in all riverCouldCall methods:
    // if (ctx.commonContext.bettingContext.potOdd > 10 && 
    //     ctx.personalContext.m_handSimulation.winRanged < .5 &&
    //     ctx.commonContext.bettingContext.highestBetAmount >= ctx.personalContext.cash + ctx.personalContext.totalBetAmount &&
    //     ctx.personalContext.m > 8)
    
    return ctx.commonContext.bettingContext.potOdd > 10.0f &&
           ctx.personalContext.m_handSimulation.winRanged < 0.5f &&
           ctx.commonContext.bettingContext.highestBetAmount >= 
               (ctx.personalContext.cash + ctx.personalContext.totalBetAmount) &&
           ctx.personalContext.m > 8.0f;
}

bool PokerMath::hasDeepStack(const CurrentHandContext& ctx, float potMultiplier)
{
    // Pattern: ctx.personalContext.cash > ctx.commonContext.bettingContext.pot * X
    return ctx.personalContext.cash > (ctx.commonContext.bettingContext.pot * potMultiplier);
}

// ========================================
// Opponent Analysis  
// ========================================

// ========================================
// Bet Sizing Calculations (Phase 2) 
// ========================================

float PokerMath::calculateValueBetSize(const CurrentHandContext& ctx, float multiplier)
{
    // Standard value bet sizing - replaces scattered (pot * 0.6), (pot * 0.75) patterns
    return ctx.commonContext.bettingContext.pot * multiplier;
}

float PokerMath::calculateBluffBetSize(const CurrentHandContext& ctx, float multiplier)
{
    // Bluff bet sizing - replaces (pot * 0.8), (pot * 0.9) patterns
    return ctx.commonContext.bettingContext.pot * multiplier;
}

float PokerMath::calculateBlockingBetSize(const CurrentHandContext& ctx)
{
    // Small blocking bet - replaces pot * 0.33, pot * 0.25 patterns
    return ctx.commonContext.bettingContext.pot * BLOCKING_BET_MULTIPLIER;
}

float PokerMath::calculateCBetSize(const CurrentHandContext& ctx, bool isBluff)
{
    // Continuation bet sizing logic that appears in multiple strategies
    if (isBluff)
    {
        return calculateBluffBetSize(ctx, STANDARD_BLUFF_MULTIPLIER);
    }
    else
    {
        return calculateValueBetSize(ctx, STANDARD_VALUE_BET_MULTIPLIER);
    }
}

// ========================================
// Multi-Street Decision Patterns (Phase 2)
// ========================================

bool PokerMath::isGoodCBetSpot(const CurrentHandContext& ctx)
{
    // Common pattern: heads-up + not too wet board + position
    // This consolidates logic scattered across multiple strategy classes
    return isHeadsUp(ctx) && 
           !tooManyOpponents(ctx, TOO_MANY_OPPONENTS_THRESHOLD) &&
           hasPositionalAdvantage(ctx);
}

bool PokerMath::tooManyOpponents(const CurrentHandContext& ctx, int threshold)
{
    // Pattern: if (ctx.commonContext.playersContext.nbPlayers > X) found everywhere
    return ctx.commonContext.playersContext.nbPlayers > threshold;
}

bool PokerMath::isHeadsUp(const CurrentHandContext& ctx)
{
    // Pattern: ctx.commonContext.playersContext.nbPlayers == 2
    return ctx.commonContext.playersContext.nbPlayers == HEADS_UP_PLAYER_COUNT;
}

bool PokerMath::hasPositionalAdvantage(const CurrentHandContext& ctx)
{
    // Simplified position check - can be enhanced based on specific position logic
    // This would need access to seating/position info from the context
    // For now, return true as placeholder - needs refinement with actual position data
    return true; // TODO: Implement actual position logic when position data is clarified
}

// ========================================
// Preflop Raise Sizing Calculations (Phase 3)
// ========================================

float PokerMath::calculateStandardOpenRaise(const CurrentHandContext& ctx)
{
    // Consolidates pattern: 2*BB (deep stack) or 1.5*BB (shallow stack)
    const int bigBlind = ctx.commonContext.smallBlind * 2;
    const float multiplier = (ctx.personalContext.m > DEEP_STACK_M_THRESHOLD) ? 
        STANDARD_OPEN_DEEP_MULTIPLIER : STANDARD_OPEN_SHALLOW_MULTIPLIER;
    
    return bigBlind * multiplier;
}

float PokerMath::calculate3BetSize(const CurrentHandContext& ctx)
{
    // Consolidates pattern: totalPot * (1.2 with position, 1.4 without position)
    const int totalPot = ctx.commonContext.bettingContext.sets;
    
    // Check if we have position over the original raiser
    bool hasPosition = false;
    if (ctx.commonContext.playersContext.preflopLastRaiser != nullptr)
    {
        hasPosition = ctx.personalContext.position > 
            ctx.commonContext.playersContext.preflopLastRaiser->getPosition();
    }
    
    const float multiplier = hasPosition ? THREE_BET_POSITION_MULTIPLIER : THREE_BET_NO_POSITION_MULTIPLIER;
    return totalPot * multiplier;
}

float PokerMath::calculate4BetPlusSize(const CurrentHandContext& ctx)
{
    // Consolidates pattern: totalPot * (1.0 with position, 1.2 without position)
    const int totalPot = ctx.commonContext.bettingContext.sets;
    
    // Check if we have position over the original raiser
    bool hasPosition = false;
    if (ctx.commonContext.playersContext.preflopLastRaiser != nullptr)
    {
        hasPosition = ctx.personalContext.position > 
            ctx.commonContext.playersContext.preflopLastRaiser->getPosition();
    }
    
    const float multiplier = hasPosition ? FOUR_BET_POSITION_MULTIPLIER : FOUR_BET_NO_POSITION_MULTIPLIER;
    return totalPot * multiplier;
}

float PokerMath::getPositionRaiseAdjustment(const CurrentHandContext& ctx)
{
    // Consolidates position adjustment logic: +BB early, -SB button
    if (ctx.commonContext.playersContext.nbPlayers > 4)
    {
        const int bigBlind = ctx.commonContext.smallBlind * 2;
        
        if (ctx.personalContext.position < Middle)
        {
            return bigBlind;  // Early position: add big blind
        }
        else if (ctx.personalContext.position == Button)
        {
            return -ctx.commonContext.smallBlind;  // Button: subtract small blind
        }
    }
    return 0.0f;  // No adjustment needed
}

float PokerMath::getLimperRaiseAdjustment(const CurrentHandContext& ctx)
{
    // Consolidates limper adjustment: +BB per limper
    const int bigBlind = ctx.commonContext.smallBlind * 2;
    return ctx.commonContext.bettingContext.preflopCallsNumber * bigBlind;
}

// ========================================
// Opponent Modeling Utilities (Phase 4)
// ========================================

float PokerMath::getOpponentAggression(const CurrentHandContext& ctx, std::shared_ptr<Player> opponent)
{
    // Default to the most relevant opponent if none specified
    if (!opponent)
    {
        switch (ctx.commonContext.gameState)
        {
        case GameState::Turn:
            opponent = ctx.commonContext.playersContext.turnLastRaiser;
            break;
        case GameState::River:
            opponent = ctx.commonContext.playersContext.lastVPIPPlayer;
            break;
        case GameState::Flop:
            opponent = ctx.commonContext.playersContext.flopLastRaiser;
            break;
        default:
            opponent = ctx.commonContext.playersContext.preflopLastRaiser;
            break;
        }
    }
    
    if (!opponent) return 0.0f;
    
    float aggression;
    int hands;
    if (getStreetStatistics(ctx, opponent, aggression, hands))
    {
        return aggression;
    }
    
    return 0.0f;  // No reliable stats available
}

bool PokerMath::hasReliableOpponentStats(const CurrentHandContext& ctx, std::shared_ptr<Player> opponent)
{
    // Use same opponent selection logic as getOpponentAggression 
    if (!opponent)
    {
        switch (ctx.commonContext.gameState)
        {
        case GameState::Turn:
            opponent = ctx.commonContext.playersContext.turnLastRaiser;
            break;
        case GameState::River:
            opponent = ctx.commonContext.playersContext.lastVPIPPlayer;
            break;
        case GameState::Flop:
            opponent = ctx.commonContext.playersContext.flopLastRaiser;
            break;
        default:
            opponent = ctx.commonContext.playersContext.preflopLastRaiser;
            break;
        }
    }
    
    if (!opponent) return false;
    
    float aggression;
    int hands;
    return getStreetStatistics(ctx, opponent, aggression, hands);
}

bool PokerMath::isOpponentTight(const CurrentHandContext& ctx, float aggressionThreshold, std::shared_ptr<Player> opponent)
{
    // Consolidates pattern: raiserStats.getAgressionFrequency() < 20
    if (!hasReliableOpponentStats(ctx, opponent)) return false;
    
    float aggression = getOpponentAggression(ctx, opponent);
    return aggression < aggressionThreshold;
}

bool PokerMath::isOpponentPassive(const CurrentHandContext& ctx, float aggressionThreshold, std::shared_ptr<Player> opponent)
{
    // Consolidates pattern: raiserStats.getAgressionFrequency() < 30
    if (!hasReliableOpponentStats(ctx, opponent)) return false;
    
    float aggression = getOpponentAggression(ctx, opponent);
    return aggression < aggressionThreshold;
}

bool PokerMath::getStreetStatistics(const CurrentHandContext& ctx, std::shared_ptr<Player> opponent, float& aggression, int& hands)
{
    // Consolidates the complex statistics retrieval pattern with fallback logic
    if (!opponent) return false;
    
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    
    // First try: current table size statistics
    auto stats = opponent->getStatisticsUpdater()->getStatistics(nbPlayers);
    
    switch (ctx.commonContext.gameState)
    {
    case GameState::Turn:
        hands = stats.turnStatistics.hands;
        if (hands >= MIN_HANDS_FOR_RELIABLE_STATS)
        {
            aggression = stats.turnStatistics.getAgressionFrequency();
            return true;
        }
        break;
    case GameState::River:
        hands = stats.riverStatistics.hands;
        if (hands >= MIN_HANDS_FOR_RELIABLE_STATS)
        {
            aggression = stats.riverStatistics.getAgressionFrequency();
            return true;
        }
        break;
    case GameState::Flop:
        hands = stats.flopStatistics.hands;
        if (hands >= MIN_HANDS_FOR_RELIABLE_STATS)
        {
            aggression = stats.flopStatistics.getAgressionFrequency();
            return true;
        }
        break;
    default:
        hands = stats.preflopStatistics.hands;
        if (hands >= MIN_HANDS_FOR_RELIABLE_STATS)
        {
            aggression = stats.preflopStatistics.getPreflopCallthreeBetsFrequency();
            return true;
        }
        break;
    }
    
    // Fallback: try nbPlayers + 1 statistics (pattern found in all strategy classes)
    if (nbPlayers < FALLBACK_PLAYER_COUNT_THRESHOLD)
    {
        auto fallbackStats = opponent->getStatisticsUpdater()->getStatistics(nbPlayers + 1);
        
        switch (ctx.commonContext.gameState)
        {
        case GameState::Turn:
            hands = fallbackStats.turnStatistics.hands;
            if (hands >= MIN_HANDS_FOR_RELIABLE_STATS)
            {
                aggression = fallbackStats.turnStatistics.getAgressionFrequency();
                return true;
            }
            break;
        case GameState::River:
            hands = fallbackStats.riverStatistics.hands;
            if (hands >= MIN_HANDS_FOR_RELIABLE_STATS)
            {
                aggression = fallbackStats.riverStatistics.getAgressionFrequency();
                return true;
            }
            break;
        case GameState::Flop:
            hands = fallbackStats.flopStatistics.hands;
            if (hands >= MIN_HANDS_FOR_RELIABLE_STATS)
            {
                aggression = fallbackStats.flopStatistics.getAgressionFrequency();
                return true;
            }
            break;
        default:
            hands = fallbackStats.preflopStatistics.hands;
            if (hands >= MIN_HANDS_FOR_RELIABLE_STATS)
            {
                aggression = fallbackStats.preflopStatistics.getPreflopCallthreeBetsFrequency();
                return true;
            }
            break;
        }
    }
    
    return false;  // No reliable statistics available
}

// ========================================
// Randomization and Probability Utilities (Phase 5)
// ========================================

bool PokerMath::shouldPerformAction(std::shared_ptr<pkt::core::ServiceContainer> serviceContainer, float probabilityPercent)
{
    // Convert percentage to 1-N range for getRand compatibility
    if (probabilityPercent <= 0.0f) return false;
    if (probabilityPercent >= 100.0f) return true;
    
    int range = static_cast<int>(100.0f / probabilityPercent);
    int rand = 0;
    // Access randomizer through service container like existing strategies do
    serviceContainer->randomizer().getRand(1, range, 1, &rand);
    return rand == 1;
}

bool PokerMath::shouldTakeAction(std::shared_ptr<pkt::core::ServiceContainer> serviceContainer, float probability)
{
    return shouldPerformAction(serviceContainer, probability * 100.0f);
}

float PokerMath::getRandomBetMultiplier(std::shared_ptr<pkt::core::ServiceContainer> serviceContainer, float minMultiplier, float maxMultiplier)
{
    // Common pattern: m_services->randomizer().getRand(40, 80, 1, &rand); float coeff = rand / 100.0f;
    int minPercent = static_cast<int>(minMultiplier * 100);
    int maxPercent = static_cast<int>(maxMultiplier * 100);
    int rand = 0;
    serviceContainer->randomizer().getRand(minPercent, maxPercent, 1, &rand);
    return static_cast<float>(rand) / 100.0f;
}

bool PokerMath::shouldDefendAgainst3Bet(std::shared_ptr<pkt::core::ServiceContainer> serviceContainer)
{
    // LAG pattern: getRand(1, 3) == 1 for defending against 3-bet bluffs
    return shouldPerformAction(serviceContainer, DEFEND_3BET_PROBABILITY);
}

bool PokerMath::shouldAddSpeculativeHand(std::shared_ptr<pkt::core::ServiceContainer> serviceContainer)
{
    // Pattern: getRand(1, 4) == 1 for adding speculative hands to range
    return shouldPerformAction(serviceContainer, SPECULATIVE_HAND_PROBABILITY);
}

bool PokerMath::shouldBluffFrequently(std::shared_ptr<pkt::core::ServiceContainer> serviceContainer)
{
    // Pattern: getRand(1, 2) == 1 for 50% bluff frequency
    return shouldPerformAction(serviceContainer, FREQUENT_BLUFF_PROBABILITY);
}

bool PokerMath::shouldBluffOccasionally(std::shared_ptr<pkt::core::ServiceContainer> serviceContainer)
{
    // Pattern: getRand(1, 4) == 1 for 25% bluff frequency  
    return shouldPerformAction(serviceContainer, OCCASIONAL_BLUFF_PROBABILITY);
}

bool PokerMath::shouldBluffRarely(std::shared_ptr<pkt::core::ServiceContainer> serviceContainer)
{
    // Pattern: getRand(1, 7) == 1 for ~14% bluff frequency
    return shouldPerformAction(serviceContainer, RARE_BLUFF_PROBABILITY);
}

bool PokerMath::shouldSlowPlay(const CurrentHandContext& ctx, std::shared_ptr<pkt::core::ServiceContainer> serviceContainer)
{
    // Pattern: getRand(1, 4) == 1 with very strong hand for slow playing
    if (!hasVeryStrongEquity(ctx, 0.98f)) return false;
    return shouldPerformAction(serviceContainer, SLOWPLAY_PROBABILITY);
}

bool PokerMath::shouldHideHandStrength(std::shared_ptr<pkt::core::ServiceContainer> serviceContainer)
{
    // Pattern: getRand(1, 6) == 1 for hiding hand strength by calling instead of raising
    return shouldPerformAction(serviceContainer, HIDE_STRENGTH_PROBABILITY);
}

int PokerMath::getRandomUTGRange(std::shared_ptr<pkt::core::ServiceContainer> serviceContainer, int minRange, int maxRange)
{
    // Strategy-dependent UTG range randomization
    int utgRange = 0;
    serviceContainer->randomizer().getRand(minRange, maxRange, 1, &utgRange);
    return utgRange;
}

} // namespace pkt::core::player