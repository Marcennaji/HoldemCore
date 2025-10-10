// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once

#include <core/player/PlayerStatistics.h>
#include <core/player/strategy/CurrentHandContext.h>

#include <string>

struct PostFlopAnalysisFlags;

// Forward declarations for the specialized classes
namespace pkt::core::player
{
class FlopPlausibilityChecker;
class TurnPlausibilityChecker;
class RiverPlausibilityChecker;
} // namespace pkt::core::player

namespace pkt::core::player
{

class CurrentHandActions;


/**
 * @brief Validates hand plausibility across all betting rounds and actions.
 * 
 * Comprehensive hand range validation system that checks whether specific
 * hand combinations are consistent with observed betting patterns throughout
 * all phases of the hand (flop, turn, river).
 *
 * This class is for pruning the range of possible hands a player may have, given his actions postflop, and his playing style. 
 * 
 * Parameters for the methods are:
 * 
 * - possibleSituation : the PostFlopAnalysisFlags that correspond to a possible hand the player may have started with preflop
 * - ctx : the CurrentHandContext of the player on who we want to check the plausibility of the possibleSituation
 * 
 * Note that the "private" data of CurrentHandContext is not used here, only the public data (statistics, position,
 * actions).
 * 
 */
class HandPlausibilityChecker
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
