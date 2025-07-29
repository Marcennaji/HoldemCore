#pragma once

#include <core/engine/model/PlayerStatistics.h>
#include <core/player/strategy/CurrentHandContext.h>

#include <string>

struct PostFlopAnalysisFlags;

namespace pkt::core::player
{

class CurrentHandActions;

class HandPlausibilityChecker
{
  public:
    static bool isUnplausibleHandGivenFlopCheck(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenFlopBet(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenFlopCall(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenFlopRaise(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenFlopAllin(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& context);

    static bool isUnplausibleHandGivenTurnCheck(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenTurnBet(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenTurnCall(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenTurnRaise(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenTurnAllin(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& context);

    static bool isUnplausibleHandGivenRiverCheck(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenRiverBet(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenRiverCall(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenRiverRaise(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenRiverAllin(const PostFlopAnalysisFlags& testedHand, CurrentHandContext& context);
};

} // namespace pkt::core::player
