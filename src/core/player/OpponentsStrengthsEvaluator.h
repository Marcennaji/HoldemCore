// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/engine/cards/Card.h"
#include "core/ports/HandEvaluationEngine.h"
#include "core/ports/Logger.h"

#include <map>
#include <string>
#include <vector>

namespace pkt::core::player
{

class Player;
struct CurrentHandContext;

/**
 * @brief Evaluates all opponents' hand strengths relative to player's current hand.
 * 
 * Analyzes each opponent's estimated range against the player's hand ranking,
 * calculating what percentage of each opponent's range beats the player.
 * This supports strategic decision-making by identifying the strongest threats.
 */
class OpponentsStrengthsEvaluator
{
  public:
    /**
     * @brief Result of opponents' strength evaluation.
     * 
     * Contains both individual opponent strengths and aggregate statistics
     * computed in a single pass for efficiency.
     */
    struct EvaluationResult
    {
        std::map<int, float> opponentStrengths;  // opponent ID → % of hands that beat us (0.0 to 1.0)
        float maxStrength;                        // Maximum strength among all opponents
        int strongestOpponentId;                  // ID of strongest opponent (-1 if none)
        
        EvaluationResult() : maxStrength(-1), strongestOpponentId(-1) {}
    };

    OpponentsStrengthsEvaluator(
        int playerId,
        core::Logger& logger,
        core::HandEvaluationEngine& handEvaluator
    );

    /**
     * @brief Evaluate all active opponents' strengths in a single pass.
     * 
     * For each active opponent:
     * 1. Gets their estimated range (computing it if needed)
     * 2. Filters out impossible hands given board and known cards
     * 3. Calculates what % of their range beats the player's hand
     * 
     * @param ctx Current hand context with player list and board state
     * @param playerHand Current player's hole cards
     * @param playerHandRanking Current player's hand ranking
     * @return Complete evaluation with individual strengths and summary stats
     */
    EvaluationResult evaluateOpponents(
        const CurrentHandContext& ctx,
        const HoleCards& playerHand,
        int playerHandRanking
    ) const;

  private:
    /**
     * @brief Calculate percentage of opponent's range that beats player's hand.
     * 
     * @param opponent Opponent player reference
     * @param playerHand Current player's hole cards  
     * @param playerHandRanking Current player's hand ranking
     * @param board Board cards as string
     * @param ctx Current hand context
     * @return Percentage of opponent's range that wins (0.0 to 1.0)
     */
    float calculateOpponentWinningPercentage(
        const Player& opponent,
        const HoleCards& playerHand,
        int playerHandRanking,
        const std::string& board,
        const CurrentHandContext& ctx
    ) const;

    /**
     * @brief Filter out impossible hands given board and known cards.
     * 
     * Removes hands that:
     * - Contain cards already on the board
     * - Contain cards the player is known to hold
     * 
     * @param ranges All possible hands in opponent's range
     * @param board Board cards as string
     * @param playerHand Player's hole cards
     * @return Vector of plausible hands
     */
    std::vector<std::string> filterPlausibleHands(
        const std::vector<std::string>& ranges,
        const std::string& board,
        const HoleCards& playerHand
    ) const;

    const int m_playerId;
    core::Logger& m_logger;
    core::HandEvaluationEngine& m_handEvaluator;
};

} // namespace pkt::core::player
