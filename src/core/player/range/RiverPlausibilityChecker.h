#pragma once
#include "core/player/strategy/CurrentHandContext.h"

struct PostFlopAnalysisFlags;

namespace pkt::core::player
{
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