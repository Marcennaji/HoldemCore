#pragma once

#include <gtest/gtest.h>
#include "core/engine/EngineDefs.h"
#include "core/engine/model/GameState.h"
#include "core/interfaces/HandEvaluationEngine.h"
#include "core/player/PlayerStatistics.h"
#include "core/player/range/HandPlausibilityChecker.h"
#include "core/player/strategy/CurrentHandContext.h"
#include "core/player/typedefs.h"

namespace pkt::test
{

using namespace pkt::core::player;
using namespace pkt::core;

/**
 * Base class providing comprehensive utilities for testing HandPlausibilityChecker methods.
 * Offers helper methods to create realistic poker scenarios with different:
 * - Hand situations (pairs, draws, made hands)
 * - Opponent profiles (tight-passive, loose-aggressive, maniac)
 * - Game contexts (heads-up, multiway, position scenarios)
 * - Board textures (dry, wet, paired boards)
 */
class HandPlausibilityTestBase : public ::testing::Test
{
  protected:
    void SetUp() override;
    void TearDown() override;

    // ========================================
    // Hand Context Creation Utilities
    // ========================================

    /**
     * Creates a basic hand context with configurable position and aggression
     * @param hasPosition Whether the player is in position
     * @param isPassive Whether the player has passive statistics
     * @return Configured CurrentHandContext
     */
    CurrentHandContext createHandContext(bool hasPosition = true, bool isPassive = false);

    /**
     * Creates hand context for multiway pot scenarios
     * @param numPlayers Number of players in the pot
     * @param hasPosition Whether the player is in position
     * @return Configured CurrentHandContext
     */
    CurrentHandContext createMultiwayContext(int numPlayers, bool hasPosition = true);

    /**
     * Creates heads-up scenario context
     * @param hasPosition Whether the player is in position
     * @return Configured CurrentHandContext
     */
    CurrentHandContext createHeadsUpContext(bool hasPosition = true);

    // ========================================
    // Player Statistics & Profile Utilities
    // ========================================

    /**
     * Sets player statistics for given context
     * @param ctx Context to modify
     * @param aggressionFactor Aggression factor (1.0 = passive, 3.0+ = aggressive)
     * @param aggressionFreq Aggression frequency percentage (0-100)
     * @param handsSample Number of hands in sample for statistical reliability
     */
    void setPlayerStatistics(CurrentHandContext& ctx, float aggressionFactor, float aggressionFreq, int handsSample);

    /**
     * Creates tight-passive player profile
     * @param ctx Context to modify
     */
    void setTightPassiveProfile(CurrentHandContext& ctx);

    /**
     * Creates loose-aggressive player profile
     * @param ctx Context to modify
     */
    void setLooseAggressiveProfile(CurrentHandContext& ctx);

    /**
     * Creates moderate aggressive player profile (allows some slowplay)
     * @param ctx Context to modify
     */
    void setModerateProfile(CurrentHandContext& ctx);

    /**
     * Creates maniac player profile (very loose and aggressive)
     * @param ctx Context to modify
     */
    void setManiacProfile(CurrentHandContext& ctx);

    /**
     * Sets insufficient statistics (not reliable for making assumptions)
     * @param ctx Context to modify
     */
    void setInsufficientStatistics(CurrentHandContext& ctx);

    // ========================================
    // Game Context & Betting Utilities
    // ========================================

    /**
     * Sets the number of players in the pot
     * @param ctx Context to modify
     * @param numPlayers Number of active players
     */
    void setMultiPlayerPot(CurrentHandContext& ctx, int numPlayers);

    /**
     * Sets betting context for aggressive action
     * @param ctx Context to modify
     * @param betsOrRaises Number of bets/raises in current round
     */
    void setAggressiveAction(CurrentHandContext& ctx, int betsOrRaises);

    /**
     * Sets betting context for passive action (checks all around)
     * @param ctx Context to modify
     */
    void setPassiveAction(CurrentHandContext& ctx);

    /**
     * Sets betting context for first to act scenario (no prior checks)
     * @param ctx Context to modify
     */
    void setFirstToAct(CurrentHandContext& ctx);

    /**
     * Sets player as aggressor on previous street
     * @param ctx Context to modify
     * @param street GameState where player was aggressor
     */
    void setPreviousAggressor(CurrentHandContext& ctx, GameState street);

    /**
     * Sets player as non-aggressor (faced bet/raise)
     * @param ctx Context to modify
     */
    void setFacingAggression(CurrentHandContext& ctx);

    // ========================================
    // Hand Analysis Creation Utilities
    // ========================================

    /**
     * Creates a basic PostFlopAnalysisFlags structure
     * @return Default PostFlopAnalysisFlags with all fields initialized to false
     */
    PostFlopAnalysisFlags createHandAnalysis();

    /**
     * Creates analysis for top pair hand
     * @param usesHoleCards Whether hand uses hole cards (true for made hands)
     * @return Configured PostFlopAnalysisFlags
     */
    PostFlopAnalysisFlags createTopPair(bool usesHoleCards = true);

    /**
     * Creates analysis for overpair hand
     * @param usesHoleCards Whether hand uses hole cards
     * @return Configured PostFlopAnalysisFlags
     */
    PostFlopAnalysisFlags createOverPair(bool usesHoleCards = true);

    /**
     * Creates analysis for middle pair
     * @param usesHoleCards Whether hand uses hole cards
     * @return Configured PostFlopAnalysisFlags
     */
    PostFlopAnalysisFlags createMiddlePair(bool usesHoleCards = true);

    /**
     * Creates analysis for two pair
     * @param usesHoleCards Whether hand uses hole cards
     * @return Configured PostFlopAnalysisFlags
     */
    PostFlopAnalysisFlags createTwoPair(bool usesHoleCards = true);

    /**
     * Creates analysis for trips/set
     * @param usesHoleCards Whether hand uses hole cards
     * @return Configured PostFlopAnalysisFlags
     */
    PostFlopAnalysisFlags createTrips(bool usesHoleCards = true);

    /**
     * Creates analysis for made straight
     * @param usesHoleCards Whether hand uses hole cards
     * @return Configured PostFlopAnalysisFlags
     */
    PostFlopAnalysisFlags createStraight(bool usesHoleCards = true);

    /**
     * Creates analysis for made flush
     * @param usesHoleCards Whether hand uses hole cards
     * @return Configured PostFlopAnalysisFlags
     */
    PostFlopAnalysisFlags createFlush(bool usesHoleCards = true);

    /**
     * Creates analysis for full house
     * @param usesHoleCards Whether hand uses hole cards
     * @return Configured PostFlopAnalysisFlags
     */
    PostFlopAnalysisFlags createFullHouse(bool usesHoleCards = true);

    /**
     * Creates analysis for quads
     * @param usesHoleCards Whether hand uses hole cards
     * @return Configured PostFlopAnalysisFlags
     */
    PostFlopAnalysisFlags createQuads(bool usesHoleCards = true);

    // ========================================
    // Draw and Weak Hand Utilities
    // ========================================

    /**
     * Creates analysis for overcards (no pair, high cards)
     * @param usesHoleCards Whether hand uses hole cards
     * @return Configured PostFlopAnalysisFlags
     */
    PostFlopAnalysisFlags createOvercards(bool usesHoleCards = true);

    /**
     * Creates analysis for flush draw
     * @param outs Number of flush outs (usually 9)
     * @param usesHoleCards Whether hand uses hole cards
     * @return Configured PostFlopAnalysisFlags
     */
    PostFlopAnalysisFlags createFlushDraw(int outs = 9, bool usesHoleCards = true);

    /**
     * Creates analysis for straight draw
     * @param outs Number of straight outs (4 for gutshot, 8 for open-ended)
     * @param usesHoleCards Whether hand uses hole cards
     * @return Configured PostFlopAnalysisFlags
     */
    PostFlopAnalysisFlags createStraightDraw(int outs = 8, bool usesHoleCards = true);

    /**
     * Creates analysis for combo draw (flush + straight)
     * @param flushOuts Number of flush outs
     * @param straightOuts Number of straight outs
     * @param usesHoleCards Whether hand uses hole cards
     * @return Configured PostFlopAnalysisFlags
     */
    PostFlopAnalysisFlags createComboDraw(int flushOuts = 9, int straightOuts = 8, bool usesHoleCards = true);

    // ========================================
    // Board Texture Utilities
    // ========================================

    /**
     * Sets board as draw-heavy (flush and straight draws possible)
     * @param hand Hand analysis to modify
     */
    void setDrawHeavyBoard(PostFlopAnalysisFlags& hand);

    /**
     * Sets board as dry (minimal draws)
     * @param hand Hand analysis to modify
     */
    void setDryBoard(PostFlopAnalysisFlags& hand);

    /**
     * Sets board as paired (full house possibilities)
     * @param hand Hand analysis to modify
     */
    void setPairedBoard(PostFlopAnalysisFlags& hand);

    /**
     * Sets board as rainbow (no flush draws)
     * @param hand Hand analysis to modify
     */
    void setRainbowBoard(PostFlopAnalysisFlags& hand);

    /**
     * Sets flush draw possible on board
     * @param hand Hand analysis to modify
     */
    void setFlushDrawBoard(PostFlopAnalysisFlags& hand);

    /**
     * Sets straight draw possible on board
     * @param hand Hand analysis to modify
     */
    void setStraightDrawBoard(PostFlopAnalysisFlags& hand);

    // ========================================
    // Validation Utilities
    // ========================================

    /**
     * Validates that hand uses hole cards appropriately
     * @param hand Hand analysis to validate
     * @param shouldUseCards Expected hole card usage
     */
    void validateHoleCardUsage(const PostFlopAnalysisFlags& hand, bool shouldUseCards);

    /**
     * Validates statistical reliability for making assumptions
     * @param ctx Context to validate
     * @param shouldBeReliable Expected reliability
     */
    void validateStatisticalReliability(const CurrentHandContext& ctx, bool shouldBeReliable);

    // ========================================
    // Constants for Testing
    // ========================================

    // Aggression thresholds based on HandPlausibilityChecker logic
    // Passive: aggressionFactor < 2 AND aggressionFrequency < 30
    static constexpr float PASSIVE_AGGRESSION_FACTOR = 1.5f;
    static constexpr float PASSIVE_AGGRESSION_FREQUENCY = 25.0f;

    // Aggressive: aggressionFactor >= 2 OR aggressionFrequency >= 30 (but still allows some slowplay)
    static constexpr float AGGRESSIVE_AGGRESSION_FACTOR = 2.5f;
    static constexpr float AGGRESSIVE_AGGRESSION_FREQUENCY = 35.0f;

    // Moderate: aggressive enough to not be passive, but moderate enough to allow some slowplay
    static constexpr float MODERATE_AGGRESSION_FACTOR = 2.2f;
    static constexpr float MODERATE_AGGRESSION_FREQUENCY = 32.0f;

    // Maniac: very aggressive (aggressionFactor > 3 AND aggressionFrequency > 50)
    static constexpr float MANIAC_AGGRESSION_FACTOR = 5.0f;
    static constexpr float MANIAC_AGGRESSION_FREQUENCY = 70.0f;

  private:
    /**
     * Creates a mock PlayerList for testing
     * @param numPlayers Number of players to create
     * @return Shared pointer to PlayerList
     */
    std::shared_ptr<std::list<std::shared_ptr<Player>>> createMockPlayerList(int numPlayers);
};
} // namespace pkt::test