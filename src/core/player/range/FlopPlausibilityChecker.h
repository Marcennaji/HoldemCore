// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once
#include "core/player/strategy/CurrentHandContext.h"

struct PostFlopAnalysisFlags;

namespace pkt::core::player
{
/**
 * @brief Validates the plausibility of hands based on flop betting actions.
 * 
 * Analyzes whether specific hand combinations are consistent with observed
 * betting patterns on the flop, helping to refine opponent range estimates
 * by eliminating unlikely hand combinations.
 */
class FlopPlausibilityChecker
{
  public:
    static bool isUnplausibleHandGivenFlopCheck(const PostFlopAnalysisFlags& possibleSituation,
                                                const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenFlopBet(const PostFlopAnalysisFlags& possibleSituation,
                                              const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenFlopCall(const PostFlopAnalysisFlags& possibleSituation,
                                               const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenFlopRaise(const PostFlopAnalysisFlags& possibleSituation,
                                                const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenFlopAllin(const PostFlopAnalysisFlags& possibleSituation,
                                                const CurrentHandContext& ctx);
};
} // namespace pkt::core::player