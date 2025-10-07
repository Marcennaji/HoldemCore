// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/interfaces/Logger.h>

#include <memory>
#include <string>
#include <vector>
namespace pkt::core
{

class Logger;

} // namespace pkt::core

namespace pkt::core::player
{
struct CurrentHandContext;

class PreflopRangeEstimator
{
  public:
    PreflopRangeEstimator(int playerId, pkt::core::Logger& logger);

    std::string computeEstimatedPreflopRange(const CurrentHandContext& ctx);

  private:
    std::string computeEstimatedPreflopRangeFromLastRaiser(const CurrentHandContext& ctx) const;
    std::string computeEstimatedPreflopRangeFromCaller(const CurrentHandContext& ctx) const;

    // Analyze player actions to count raises and calls
    void analyzePlayerActions(const CurrentHandContext& ctx, int& raises, int& calls) const;

    // Calculate the starting range based on player statistics
    float calculateStartingRange(const CurrentHandContext& ctx, int nbPlayers) const;

    // Adjust the range based on the player's position
    float adjustRangeForPosition(const CurrentHandContext& ctx, int nbPlayers, const float currentRange) const;

    // Adjust the range based on pot odds
    float adjustRangeForPotOdds(const CurrentHandContext& ctx, const float currentRange) const;

    // Adjust the range based on the number of preflop raises
    float adjustRangeForPreflopRaises(const CurrentHandContext& ctx, int raises, const float currentRange) const;

    // Handle adjustments for loose/aggressive opponents
    float handleLooseAggressiveOpponents(const CurrentHandContext& ctx, int raises, const float currentRange) const;

    // Finalize the range by adding an error margin and clamping it
    float finalizeRange(const float currentRange) const;

    // Finalize the estimated range and determine if it's a top range or a filled range
    std::string finalizeEstimatedRange(const CurrentHandContext& ctx, std::vector<std::string>& ranges,
                                       std::vector<float>& rangesValues, float range, int nbPlayers,
                                       bool isTopRange) const;

    float handleLimpRange(const CurrentHandContext& ctx, const float currentRange) const;
    float handleSingleRaiseRange(const CurrentHandContext& ctx, const float currentRange) const;
    float handleThreeBetRange(const CurrentHandContext& ctx, int opponentRaises, const float currentRange) const;
    float handleFourBetOrMoreRange(const CurrentHandContext& ctx, int opponentRaises, const float currentRange) const;

  private:

    pkt::core::Logger& m_logger;

    int m_playerId;
};
} // namespace pkt::core::player