// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <string>
#include <vector>
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
    PreflopRangeEstimator(IHand* hand, int playerId);

    std::string computeEstimatedPreflopRange(CurrentHandContext& ctx);
    std::string computeEstimatedPreflopRangeFromLastRaiser(CurrentHandContext& ctx) const;
    std::string computeEstimatedPreflopRangeFromCaller(CurrentHandContext& ctx) const;

    std::string computeEstimatedPreflopRangeFromCaller_old(CurrentHandContext& ctx) const;

  private:
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

    IHand* myHand;
    int myPlayerId;
};
} // namespace pkt::core::player