#pragma once

#include <core/player/PlayerStatistics.h>
#include <core/player/strategy/CurrentHandContext.h>

#include <string>

struct PostFlopAnalysisFlags;

namespace pkt::core::player
{

class CurrentHandActions;

class HandPlausibilityChecker
{
  public:
    static bool isUnplausibleHandGivenFlopCheck(const PostFlopAnalysisFlags& testedHand, const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenFlopBet(const PostFlopAnalysisFlags& testedHand, const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenFlopCall(const PostFlopAnalysisFlags& testedHand, const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenFlopRaise(const PostFlopAnalysisFlags& testedHand, const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenFlopAllin(const PostFlopAnalysisFlags& testedHand, const CurrentHandContext& ctx);

    static bool isUnplausibleHandGivenTurnCheck(const PostFlopAnalysisFlags& testedHand, const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenTurnBet(const PostFlopAnalysisFlags& testedHand, const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenTurnCall(const PostFlopAnalysisFlags& testedHand, const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenTurnRaise(const PostFlopAnalysisFlags& testedHand, const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenTurnAllin(const PostFlopAnalysisFlags& testedHand, const CurrentHandContext& ctx);

    static bool isUnplausibleHandGivenRiverCheck(const PostFlopAnalysisFlags& testedHand,
                                                 const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenRiverBet(const PostFlopAnalysisFlags& testedHand, const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenRiverCall(const PostFlopAnalysisFlags& testedHand, const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenRiverRaise(const PostFlopAnalysisFlags& testedHand,
                                                 const CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenRiverAllin(const PostFlopAnalysisFlags& testedHand,
                                                 const CurrentHandContext& ctx);
};

} // namespace pkt::core::player
