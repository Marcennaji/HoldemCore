// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/engine/EngineDefs.h>
#include <core/engine/model/GameState.h>
#include <core/player/PlayerStatistics.h>
#include <core/interfaces/Logger.h>
#include <core/interfaces/HandEvaluationEngine.h>
#include "PreflopRangeEstimator.h"

#include <memory>
#include <string>
#include <vector>

struct PostFlopAnalysisFlags;

namespace pkt::core
{


struct PreflopStatistics;
struct FlopStatistics;
struct TurnStatistics;
struct RiverStatistics;
} // namespace pkt::core

namespace pkt::core::player
{
class Player;
struct CurrentHandContext;

/**
 * @brief Estimates opponent hand ranges based on betting patterns and statistics.
 * 
 * Analyzes player actions and statistics to estimate the likely range of hands
 * an opponent might hold, supporting both preflop and postflop range estimation
 * for strategic decision making.
 */
class RangeEstimator
{
  public:
    RangeEstimator(int playerId, pkt::core::Logger& logger, 
                   pkt::core::HandEvaluationEngine& handEvaluator);
    
    void setEstimatedRange(const std::string& range);
    std::string getEstimatedRange() const;

    void computeEstimatedPreflopRange(const CurrentHandContext&);
    void updateUnplausibleRanges(pkt::core::GameState, const CurrentHandContext&);
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
    
    pkt::core::Logger* m_logger;
    pkt::core::HandEvaluationEngine* m_handEvaluator;

    std::unique_ptr<PreflopRangeEstimator> m_preflopRangeEstimator;
    std::string m_estimatedRange;
    int m_playerId;
};
} // namespace pkt::core::player