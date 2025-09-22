// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/engine/EngineDefs.h>
#include <core/engine/model/GameState.h>
#include <core/player/PlayerStatistics.h>
#include <core/services/ServiceContainer.h>
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
    explicit RangeEstimator(int playerId, std::shared_ptr<pkt::core::ServiceContainer> serviceContainer);

    void setEstimatedRange(const std::string& range);
    std::string getEstimatedRange() const;

    void computeEstimatedPreflopRange(const CurrentHandContext&);
    void updateUnplausibleRanges(GameState, const CurrentHandContext&);
    static int getStandardRaisingRange(int nbPlayers);
    static int getStandardCallingRange(int nbPlayers);

    static std::string getStringRange(int nbPlayers, int range);
    static std::string getFilledRange(std::vector<std::string>& ranges, std::vector<float>& rangesValues,
                                      const float rangeMax, int nbPlayers);

  private:
    void updateUnplausibleRangesGivenPreflopActions(const CurrentHandContext&);
    void updateUnplausibleRangesGivenFlopActions(const CurrentHandContext&);
    void updateUnplausibleRangesGivenTurnActions(const CurrentHandContext&);
    void updateUnplausibleRangesGivenRiverActions(const CurrentHandContext&);
    void ensureServicesInitialized() const;
    mutable std::shared_ptr<pkt::core::ServiceContainer> myServices; // Injected service container

    std::unique_ptr<PreflopRangeEstimator> myPreflopRangeEstimator;
    std::string myEstimatedRange;
    int myPlayerId;
};
} // namespace pkt::core::player