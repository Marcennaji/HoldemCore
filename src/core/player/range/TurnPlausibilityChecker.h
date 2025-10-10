// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once
#include "core/player/strategy/CurrentHandContext.h"

struct PostFlopAnalysisFlags;

namespace pkt::core::player
{
/**
 * @brief Validates the plausibility of hands based on turn betting actions.
 * 
 * Analyzes whether specific hand combinations are consistent with observed
 * betting patterns on the turn, helping to further refine opponent range
 * estimates as more information becomes available.
 */
class TurnPlausibilityChecker
{
  public:
    static bool isUnplausibleHandGivenTurnCheck(const PostFlopAnalysisFlags& possibleSituation,
                                                const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenTurnBet(const PostFlopAnalysisFlags& possibleSituation,
                                              const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenTurnCall(const PostFlopAnalysisFlags& possibleSituation,
                                               const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenTurnRaise(const PostFlopAnalysisFlags& possibleSituation,
                                                const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenTurnAllin(const PostFlopAnalysisFlags& possibleSituation,
                                                const CurrentHandContext& ctx);
};
} // namespace pkt::core::player