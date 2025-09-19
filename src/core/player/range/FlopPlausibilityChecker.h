#pragma once
#include "core/player/strategy/CurrentHandContext.h"

struct PostFlopAnalysisFlags;

namespace pkt::core::player
{
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