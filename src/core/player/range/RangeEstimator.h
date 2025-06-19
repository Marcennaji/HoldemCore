// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/engine/EngineDefs.h>
#include <core/engine/model/PlayerStatistics.h>
#include "PreflopRangeEstimator.h"

#include <memory>
#include <string>
#include <vector>

struct PostFlopState;

namespace pkt::core
{
class IHand;

struct PreflopStatistics;
struct FlopStatistics;
struct TurnStatistics;
struct RiverStatistics;
} // namespace pkt::core

namespace pkt::core::player
{
class Player;
struct CurrentHandContext;

class RangeEstimator
{
  public:
    RangeEstimator(int playerId);

    void setHand(IHand* hand);
    void setEstimatedRange(const std::string& range);
    std::string getEstimatedRange() const;

    void computeEstimatedPreflopRange(CurrentHandContext&);

    static int getStandardRaisingRange(int nbPlayers);
    static int getStandardCallingRange(int nbPlayers);

    static std::string getStringRange(int nbPlayers, int range);
    static std::string getFilledRange(std::vector<std::string>& ranges, std::vector<float>& rangesValues,
                                      const float rangeMax, int nbPlayers);

    void updateUnplausibleRangesGivenPreflopActions(CurrentHandContext&);
    void updateUnplausibleRangesGivenFlopActions(CurrentHandContext&);
    void updateUnplausibleRangesGivenTurnActions(CurrentHandContext&);
    void updateUnplausibleRangesGivenRiverActions(CurrentHandContext&);

  private:
    std::unique_ptr<PreflopRangeEstimator> myPreflopRangeEstimator;
    std::string myEstimatedRange;
    int myPlayerId;
    IHand* myHand;
};
} // namespace pkt::core::player