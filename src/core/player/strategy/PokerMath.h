// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <memory>  // For std::shared_ptr
#include <core/interfaces/Randomizer.h>

// Forward declare ServiceContainer to avoid circular dependencies
namespace pkt::core {
    class ServiceContainer;
}

namespace pkt::core::player
{
// Forward declarations
struct CurrentHandContext;
class Player;

/**
 * @brief Common poker mathematics and decision logic shared across all strategies
 * 
 * This utility class contains the most commonly duplicated logic found across
 * TightAggressiveBotStrategy, LooseAggressiveBotStrategy, UltraTightBotStrategy, 
 * and ManiacBotStrategy. By centralizing this logic, we eliminate code duplication
 * while preserving each strategy's unique decision-making characteristics.
 */
class PokerMath
{
public:
    // ========================================
    // Equity and Odds Calculations
    // ========================================
    
    /**
     * Check if hand equity justifies calling given pot odds
     * Common pattern: ctx.personalContext.m_handSimulation.winRanged * 100 < ctx.commonContext.bettingContext.potOdd
     */
    static bool hasInsufficientEquityForCall(const CurrentHandContext& ctx, float equityMultiplier = 1.0f);
    
    /**
     * Check if hand has very strong equity (commonly used threshold: >0.93-0.95)
     */
    static bool hasVeryStrongEquity(const CurrentHandContext& ctx, float threshold = 0.93f);
    
    /**
     * Check if hand has strong equity against multiple opponents
     */
    static bool hasStrongEquityVsField(const CurrentHandContext& ctx, float threshold = 0.8f);
    
    /**
     * Check if equity is too weak to continue (common threshold: <0.25-0.3)
     */
    static bool hasWeakEquity(const CurrentHandContext& ctx, float threshold = 0.25f);

    // ========================================
    // Stack Protection Logic
    // ========================================
    
    /**
     * Check if calling would risk too much of stack relative to pot odds and equity
     * Common pattern across strategies for stack preservation
     */
    static bool wouldRiskTooMuchOfStack(const CurrentHandContext& ctx);
    
    /**
     * Check if player has sufficient stack relative to pot for aggressive action
     */
    static bool hasDeepStack(const CurrentHandContext& ctx, float potMultiplier = 4.0f);

    // ========================================
    // Opponent Analysis  
    // ========================================
    
    // NOTE: Opponent analysis methods moved to Phase 4 section below

    // ========================================
    // Bet Sizing Utilities
    // ========================================
    
    /**
     * Calculate standard value bet size (common pattern: pot * 0.6)
     */
    static float calculateValueBetSize(const CurrentHandContext& ctx, float multiplier = STANDARD_VALUE_BET_MULTIPLIER);
    
    /**
     * Calculate bluff bet size (common pattern: pot * 0.6 to 0.8)
     */
    static float calculateBluffBetSize(const CurrentHandContext& ctx, float multiplier = STANDARD_BLUFF_MULTIPLIER);
    
    /**
     * Calculate blocking bet size (common pattern: pot * 0.33)
     */
    static float calculateBlockingBetSize(const CurrentHandContext& ctx);
    
    /**
     * Calculate continuation bet size based on opponents and position
     */
    static float calculateCBetSize(const CurrentHandContext& ctx, bool isBluff = false);

    // ========================================
    // Multi-Street Decision Patterns
    // ========================================
    
    /**
     * Check if this is a good spot for continuation betting (common pattern)
     */
    static bool isGoodCBetSpot(const CurrentHandContext& ctx);
    
    /**
     * Check if opponent count suggests caution (common threshold: 4+ opponents)
     */
    static bool tooManyOpponents(const CurrentHandContext& ctx, int threshold = 4);
    
    /**
     * Check for heads-up situation (exactly 2 players)
     */
    static bool isHeadsUp(const CurrentHandContext& ctx);
    
    /**
     * Check if we have positional advantage (placeholder - needs refinement)
     */
    static bool hasPositionalAdvantage(const CurrentHandContext& ctx);

    // ========================================
    // Preflop Raise Sizing (Phase 3)
    // ========================================
    
    /**
     * Calculate standard opening raise size based on stack depth 
     * Pattern: 2*BB (deep) or 1.5*BB (shallow)
     */
    static float calculateStandardOpenRaise(const CurrentHandContext& ctx);
    
    /**
     * Calculate 3-bet size based on position relative to original raiser
     * Pattern: pot * 1.2 (position) or pot * 1.4 (no position)
     */
    static float calculate3BetSize(const CurrentHandContext& ctx);
    
    /**
     * Calculate 4-bet and higher sizes
     * Pattern: pot * 1.0 (position) or pot * 1.2 (no position)  
     */
    static float calculate4BetPlusSize(const CurrentHandContext& ctx);
    
    /**
     * Position-based raise adjustments (early +BB, button -SB)
     */
    static float getPositionRaiseAdjustment(const CurrentHandContext& ctx);
    
    /**
     * Limper-based raise adjustments (+BB per limper)
     */
    static float getLimperRaiseAdjustment(const CurrentHandContext& ctx);

    // ========================================
    // Opponent Modeling Utilities (Phase 4)
    // ========================================
    
    /**
     * Get opponent aggression frequency for current street with reliability check
     */
    static float getOpponentAggression(const CurrentHandContext& ctx, std::shared_ptr<Player> opponent = nullptr);
    
    /**
     * Check if opponent statistics are reliable (enough hands played)
     */
    static bool hasReliableOpponentStats(const CurrentHandContext& ctx, std::shared_ptr<Player> opponent = nullptr);
    
    /**
     * Determine if opponent is tight/passive based on aggression thresholds
     */
    static bool isOpponentTight(const CurrentHandContext& ctx, float aggressionThreshold = 20.0f, std::shared_ptr<Player> opponent = nullptr);
    
    /**
     * Determine if opponent is passive (low aggression frequency)
     */
    static bool isOpponentPassive(const CurrentHandContext& ctx, float aggressionThreshold = 30.0f, std::shared_ptr<Player> opponent = nullptr);
    
    /**
     * Get appropriate statistics for current street from opponent
     */
    static bool getStreetStatistics(const CurrentHandContext& ctx, std::shared_ptr<Player> opponent, float& aggression, int& hands);

    // ========================================
    // Randomization and Probability Utilities (Phase 5)
    // ========================================
    
    /**
     * Generic probability check - replaces getRand(1, X) == Y patterns
     * @param randomizer Randomizer service for probability calculations  
     * @param probabilityPercent Probability as percentage (e.g., 50 for 50% chance)
     */
    static bool shouldPerformAction(pkt::core::Randomizer& randomizer, float probabilityPercent);
    
    /**
     * Check if should perform action based on specific odds (like 1 in 6 chance)
     * Common pattern: getRand(1, N) == 1 -> shouldTakeAction(1.0f/N)
     */
    static bool shouldTakeAction(pkt::core::Randomizer& randomizer, float probability);
    
    /**
     * Get random bet coefficient for dynamic sizing
     * Common pattern: getRand(40, 80) / 100 -> getRandomBetMultiplier(0.4f, 0.8f)
     */
    static float getRandomBetMultiplier(pkt::core::Randomizer& randomizer, float minMultiplier = 0.4f, float maxMultiplier = 0.8f);
    
    /**
     * Check if should defend against 3-bet based on hand type and strategy
     * Common LAG pattern: getRand(1, 3) == 1 for 33% chance
     */
    static bool shouldDefendAgainst3Bet(pkt::core::Randomizer& randomizer);
    
    /**
     * Check if should add speculative hands to range
     * Common pattern: getRand(1, 4) == 1 for 25% chance  
     */
    static bool shouldAddSpeculativeHand(pkt::core::Randomizer& randomizer);
    
    /**
     * Check if should bluff in specific situations
     * Common patterns: getRand(1, 2) == 1 for 50%, getRand(1, 4) == 1 for 25%
     */
    static bool shouldBluffFrequently(pkt::core::Randomizer& randomizer);  // 50% chance
    static bool shouldBluffOccasionally(pkt::core::Randomizer& randomizer); // 25% chance
    static bool shouldBluffRarely(pkt::core::Randomizer& randomizer);       // ~15% chance
    
    /**
     * Check if should slowplay very strong hand
     * Common pattern: getRand(1, 4) == 1 with strong equity conditions
     */
    static bool shouldSlowPlay(const CurrentHandContext& ctx, pkt::core::Randomizer& randomizer);
    
    /**
     * Check if should hide hand strength by calling instead of raising
     * Common pattern: getRand(1, 6) == 1 for ~17% chance
     */
    static bool shouldHideHandStrength(pkt::core::Randomizer& randomizer);
    
    /**
     * Get random range multiplier for UTG position (strategy-dependent)
     * Replaces specific getRand patterns for starting range calculation
     */
    static int getRandomUTGRange(pkt::core::Randomizer& randomizer, int minRange, int maxRange);

    // ========================================
    // Constants (formerly magic numbers)
    // ========================================
    
    // Equity thresholds
    static constexpr float VERY_STRONG_EQUITY_THRESHOLD = 0.93f;
    static constexpr float STRONG_EQUITY_THRESHOLD = 0.8f;
    static constexpr float WEAK_EQUITY_THRESHOLD = 0.25f;
    static constexpr float PASSIVE_AGGRESSION_THRESHOLD = 30.0f;
    static constexpr float DEEP_STACK_POT_MULTIPLIER = 4.0f;
    static constexpr int MIN_HANDS_FOR_RELIABLE_STATS = 20;
    
    // Bet sizing multipliers
    static constexpr float STANDARD_VALUE_BET_MULTIPLIER = 0.6f;
    static constexpr float STANDARD_BLUFF_MULTIPLIER = 0.8f;
    static constexpr float BLOCKING_BET_MULTIPLIER = 0.33f;
    
    // Player count thresholds  
    static constexpr int TOO_MANY_OPPONENTS_THRESHOLD = 4;
    static constexpr int HEADS_UP_PLAYER_COUNT = 2;
    
    // Preflop raise sizing multipliers (Phase 3)
    static constexpr float STANDARD_OPEN_DEEP_MULTIPLIER = 2.0f;    // 2 * BB when M > 8
    static constexpr float STANDARD_OPEN_SHALLOW_MULTIPLIER = 1.5f; // 1.5 * BB when M <= 8
    static constexpr float THREE_BET_POSITION_MULTIPLIER = 1.2f;    // 3bet with position
    static constexpr float THREE_BET_NO_POSITION_MULTIPLIER = 1.4f; // 3bet without position
    static constexpr float FOUR_BET_POSITION_MULTIPLIER = 1.0f;     // 4bet+ with position  
    static constexpr float FOUR_BET_NO_POSITION_MULTIPLIER = 1.2f;  // 4bet+ without position
    static constexpr int DEEP_STACK_M_THRESHOLD = 8;
    
    // Opponent modeling thresholds (Phase 4)
    static constexpr float TIGHT_OPPONENT_AGGRESSION_THRESHOLD = 20.0f;   // Below this = tight
    static constexpr float PASSIVE_OPPONENT_AGGRESSION_THRESHOLD = 30.0f; // Below this = passive 
    static constexpr int FALLBACK_PLAYER_COUNT_THRESHOLD = 10;             // Use +1 fallback when < 10 players
    
    // Randomization probability thresholds (Phase 5)
    static constexpr float DEFEND_3BET_PROBABILITY = 33.33f;       // 1 in 3 chance (getRand(1,3) == 1)
    static constexpr float SPECULATIVE_HAND_PROBABILITY = 25.0f;   // 1 in 4 chance (getRand(1,4) == 1) 
    static constexpr float FREQUENT_BLUFF_PROBABILITY = 50.0f;     // 1 in 2 chance (getRand(1,2) == 1)
    static constexpr float OCCASIONAL_BLUFF_PROBABILITY = 25.0f;   // 1 in 4 chance (getRand(1,4) == 1)
    static constexpr float RARE_BLUFF_PROBABILITY = 14.29f;        // 1 in 7 chance (getRand(1,7) == 1)
    static constexpr float SLOWPLAY_PROBABILITY = 25.0f;           // 1 in 4 chance (getRand(1,4) == 1)
    static constexpr float HIDE_STRENGTH_PROBABILITY = 16.67f;     // 1 in 6 chance (getRand(1,6) == 1)
    
    // Default bet multiplier ranges for coefficient randomization
    static constexpr float MIN_BET_MULTIPLIER = 0.4f;              // 40% of pot
    static constexpr float MAX_BET_MULTIPLIER = 0.8f;              // 80% of pot
    static constexpr float MANIAC_MAX_BET_MULTIPLIER = 0.9f;       // 90% of pot for maniac strategy
};

} // namespace pkt::core::player