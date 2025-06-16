#pragma once

#include <core/engine/model/PlayerStatistics.h>
#include <core/player/strategy/CurrentHandContext.h>
#include <third_party/psim/psim.hpp>

#include <string>

struct PostFlopState;

namespace pkt::core::player
{

class CurrentHandActions;

class HandPlausibilityChecker
{
  public:
    static bool isUnplausibleHandGivenFlopCheck(const PostFlopState& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenFlopBet(const PostFlopState& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenFlopCall(const PostFlopState& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenFlopRaise(const PostFlopState& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenFlopAllin(const PostFlopState& testedHand, CurrentHandContext& context);

    static bool isUnplausibleHandGivenTurnCheck(const PostFlopState& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenTurnBet(const PostFlopState& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenTurnCall(const PostFlopState& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenTurnRaise(const PostFlopState& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenTurnAllin(const PostFlopState& testedHand, CurrentHandContext& context);

    static bool isUnplausibleHandGivenRiverCheck(const PostFlopState& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenRiverBet(const PostFlopState& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenRiverCall(const PostFlopState& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenRiverRaise(const PostFlopState& testedHand, CurrentHandContext& context);
    static bool isUnplausibleHandGivenRiverAllin(const PostFlopState& testedHand, CurrentHandContext& context);
};

} // namespace pkt::core::player
