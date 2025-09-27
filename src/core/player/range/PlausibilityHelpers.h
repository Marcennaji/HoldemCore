#pragma once
#include "core/player/strategy/CurrentHandContext.h"

struct PostFlopAnalysisFlags;

namespace pkt::core::player
{
/**
 * @brief Shared helper methods for plausibility checking across all betting rounds
 *
 * This class provides common utility methods that are used by FlopPlausibilityChecker,
 * TurnPlausibilityChecker, and RiverPlausibilityChecker to avoid code duplication
 * and maintain consistent logic across different betting rounds.
 */
class PlausibilityHelpers
{
  public:
    // Player behavior pattern helpers
    static bool isManiacPlayer(const CurrentHandContext& ctx);
    static bool isInPosition(const CurrentHandContext& ctx);
    static bool isMultiwayPot(const CurrentHandContext& ctx);
    static bool isDonkBet(const CurrentHandContext& ctx);

    // Hand strength evaluation helpers
    static bool handDoesNotUseHoleCards(const PostFlopAnalysisFlags& hand);
    static bool hasStrongHand(const PostFlopAnalysisFlags& hand);
    static bool hasStrongDraw(const PostFlopAnalysisFlags& hand);
    static bool hasWeakHand(const PostFlopAnalysisFlags& hand);
    static bool hasTopPairOrBetter(const PostFlopAnalysisFlags& hand);
    static bool hasTwoPairOrBetter(const PostFlopAnalysisFlags& hand);
    
    // Additional hand strength helpers
    static bool hasPremiumHand(const PostFlopAnalysisFlags& hand);
    static bool hasGoodDraw(const PostFlopAnalysisFlags& hand);
    static bool hasCallingWorthyHand(const PostFlopAnalysisFlags& hand);
    static bool hasRaiseWorthyHandAfterAction(const PostFlopAnalysisFlags& hand);
    static bool hasWeakPairOnPairedBoard(const PostFlopAnalysisFlags& hand);

    // Board texture helpers
    static bool isDangerousBoard(const PostFlopAnalysisFlags& hand);
    static bool isPairedBoard(const PostFlopAnalysisFlags& hand);

    // Statistics helpers
    static bool hasInsufficientStatistics(const CurrentHandContext& ctx);
    static bool isPassivePlayer(const CurrentHandContext& ctx);
    static bool isLoosePlayer(const CurrentHandContext& ctx);
    static bool isModeratelyLoosePlayer(const CurrentHandContext& ctx);

    // Betting round specific helpers
    static bool isAggressiveOnRound(const CurrentHandContext& ctx, const std::string& round);
};

} // namespace pkt::core::player