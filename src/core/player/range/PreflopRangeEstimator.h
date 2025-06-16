// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <string>

namespace pkt::core
{
class IHand;
class ILogger;

} // namespace pkt::core

namespace pkt::core::player
{
struct CurrentHandContext;

class PreflopRangeEstimator
{
  public:
    PreflopRangeEstimator(IHand* hand, ILogger* logger, int playerId);

    std::string computeEstimatedPreflopRange(CurrentHandContext& ctx);
    std::string computeEstimatedPreflopRangeFromLastRaiser(CurrentHandContext& ctx) const;
    std::string computeEstimatedPreflopRangeFromCaller(CurrentHandContext& ctx) const;

  private:
    IHand* myHand;
    ILogger* myLogger;
    int myPlayerId;
};
} // namespace pkt::core::player