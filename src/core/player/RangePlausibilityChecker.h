#pragma once

#include <core/engine/model/PlayerStatistics.h>
#include <core/player/strategy/CurrentHandContext.h>
#include <third_party/psim/psim.hpp>

#include <string>

struct PostFlopState;

namespace pkt::core::player
{

class CurrentHandActions;

class RangePlausibilityChecker
{
  public:
    bool isUnplausibleHandGivenFlopCheck(CurrentHandContext& context);
    bool isUnplausibleHandGivenFlopBet(CurrentHandContext& context);
    bool isUnplausibleHandGivenFlopCall(CurrentHandContext& context);
    bool isUnplausibleHandGivenFlopRaise(CurrentHandContext& context);
    bool isUnplausibleHandGivenFlopAllin(CurrentHandContext& context);

    bool isUnplausibleHandGivenTurnCheck(CurrentHandContext& context);
    bool isUnplausibleHandGivenTurnBet(CurrentHandContext& context);
    bool isUnplausibleHandGivenTurnCall(CurrentHandContext& context);
    bool isUnplausibleHandGivenTurnRaise(CurrentHandContext& context);
    bool isUnplausibleHandGivenTurnAllin(CurrentHandContext& context);

    bool isUnplausibleHandGivenRiverCheck(CurrentHandContext& context);
    bool isUnplausibleHandGivenRiverBet(CurrentHandContext& context);
    bool isUnplausibleHandGivenRiverCall(CurrentHandContext& context);
    bool isUnplausibleHandGivenRiverRaise(CurrentHandContext& context);
    bool isUnplausibleHandGivenRiverAllin(CurrentHandContext& context);
};

} // namespace pkt::core::player
