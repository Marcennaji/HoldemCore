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

/*
This class is for pruning the range of possible hands a player may have, given his actions postflop, and his playing
style. Parameters:
- possibleSituation : the PostFlopAnalysisFlags that correspond to a possible hand the player may have started with
preflop
- ctx : the CurrentHandContext of the player on who we want to check the plausibility of the possibleSituation
Note that the *private" data of CurrentHandContext is not used here, only the public data (statistics, position,
actions).

Returns : true if the possibleSituation is unplausible given the actions of the player and his playing style
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
