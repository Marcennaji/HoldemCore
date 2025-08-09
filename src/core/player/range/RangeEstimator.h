// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/engine/EngineDefs.h>
#include <core/player/PlayerStatistics.h>
#include "PreflopRangeEstimator.h"

#include <memory>
#include <string>
#include <vector>

struct PostFlopAnalysisFlags;

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

    void setEstimatedRange(const std::string& range);
    std::string getEstimatedRange() const;

    void computeEstimatedPreflopRange(const CurrentHandContext&);

    static int getStandardRaisingRange(int nbPlayers);
    static int getStandardCallingRange(int nbPlayers);

    static std::string getStringRange(int nbPlayers, int range);
    static std::string getFilledRange(std::vector<std::string>& ranges, std::vector<float>& rangesValues,
                                      const float rangeMax, int nbPlayers);

    void updateUnplausibleRangesGivenPreflopActions(const CurrentHandContext&);
    void updateUnplausibleRangesGivenFlopActions(const CurrentHandContext&);
    void updateUnplausibleRangesGivenTurnActions(const CurrentHandContext&);
    void updateUnplausibleRangesGivenRiverActions(const CurrentHandContext&);

  private:
    std::unique_ptr<PreflopRangeEstimator> myPreflopRangeEstimator;
    std::string myEstimatedRange;
    int myPlayerId;
};
} // namespace pkt::core::player