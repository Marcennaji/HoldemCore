// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once
#include "core/player/strategy/CurrentHandContext.h"

struct PostFlopAnalysisFlags;

namespace pkt::core::player
{
/**
 * @brief Validates the plausibility of hands based on river betting actions.
 * 
 * Analyzes whether specific hand combinations are consistent with observed
 * betting patterns on the river, providing final range refinement based
 * on end-game betting behavior and hand strength considerations.
 */
class RiverPlausibilityChecker
{
  public:
    static bool isUnplausibleHandGivenRiverCheck(const PostFlopAnalysisFlags& possibleSituation,
                                                 const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenRiverBet(const PostFlopAnalysisFlags& possibleSituation,
                                               const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenRiverCall(const PostFlopAnalysisFlags& possibleSituation,
                                                const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenRiverRaise(const PostFlopAnalysisFlags& possibleSituation,
                                                 const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenRiverAllin(const PostFlopAnalysisFlags& possibleSituation,
                                                 const CurrentHandContext& ctx);
};
} // namespace pkt::core::player