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
    static bool isUnplausibleHandGivenFlopCheck(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenFlopBet(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenFlopCall(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenFlopRaise(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenFlopAllin(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& ctx);

    static bool isUnplausibleHandGivenTurnCheck(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenTurnBet(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenTurnCall(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenTurnRaise(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenTurnAllin(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& ctx);

    static bool isUnplausibleHandGivenRiverCheck(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenRiverBet(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenRiverCall(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenRiverRaise(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& ctx);
    static bool isUnplausibleHandGivenRiverAllin(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& ctx);
};

} // namespace pkt::core::player
