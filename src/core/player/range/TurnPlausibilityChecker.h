#pragma once
#include "core/player/strategy/CurrentHandContext.h"

struct PostFlopAnalysisFlags;

namespace pkt::core::player
{
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