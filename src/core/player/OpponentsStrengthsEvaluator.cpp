// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "OpponentsStrengthsEvaluator.h"

#include "Helpers.h"
#include "Player.h"
#include "core/engine/model/Ranges.h"
#include "core/engine/utils/ExceptionUtils.h"
#include "core/player/range/RangeParser.h"
#include "core/player/strategy/CurrentHandContext.h"

#include <algorithm>
#include <cassert>

namespace pkt::core::player
{

using namespace std;

OpponentsStrengthsEvaluator::OpponentsStrengthsEvaluator(int playerId, core::Logger& logger,
                                                         core::HandEvaluationEngine& handEvaluator)
    : m_playerId(playerId), m_logger(logger), m_handEvaluator(handEvaluator)
{
}

OpponentsStrengthsEvaluator::EvaluationResult
OpponentsStrengthsEvaluator::evaluateOpponents(const CurrentHandContext& ctx, const HoleCards& playerHand,
                                               int playerHandRanking) const
{
    EvaluationResult result;

    const PlayerList& players = ctx.commonContext.playersContext.actingPlayersList;

    for (PlayerListIterator it = players->begin(); it != players->end(); ++it)
    {
        const Player& opponent = **it;

        // Skip ourselves and players who are not active
        if (opponent.getId() == m_playerId || opponent.getLastAction().type == ActionType::Fold ||
            opponent.getLastAction().type == ActionType::None)
        {
            continue;
        }

        const float opponentStrength = calculateOpponentWinningPercentage(opponent, playerHand, playerHandRanking,
                                                                          ctx.commonContext.stringBoard, ctx);

        if (opponentStrength > 1.0f)
        {
            pkt::core::utils::throwLogicError("Opponent strength calculation error: strength > 1.0 (" +
                                              std::to_string(opponentStrength) + ") for opponent " +
                                              std::to_string(opponent.getId()));
        }

        result.opponentStrengths[opponent.getId()] = opponentStrength;

        // Track strongest opponent
        if (opponentStrength > result.maxStrength)
        {
            result.maxStrength = opponentStrength;
            result.strongestOpponentId = opponent.getId();
        }
    }

    return result;
}

float OpponentsStrengthsEvaluator::calculateOpponentWinningPercentage(const Player& opponent,
                                                                      const HoleCards& playerHand,
                                                                      int playerHandRanking, const std::string& board,
                                                                      const CurrentHandContext& ctx) const
{
    if (playerHandRanking <= 0)
    {
        pkt::core::utils::throwInvalidArgument(
            "Invalid player hand ranking (" + std::to_string(playerHandRanking) +
            "). Player may not have hole cards yet. Player hand: " + playerHand.toString() + ", Board: " + board);
    }

    // Ensure opponent has an estimated range
    if (opponent.getRangeEstimator()->getEstimatedRange().empty())
    {
        // Need to compute range - note: this modifies opponent state
        opponent.getRangeEstimator()->computeEstimatedPreflopRange(ctx);
    }

    // Get all possible hands in opponent's estimated range
    vector<string> allRanges = RangeParser::getRangeAtomicValues(opponent.getRangeEstimator()->getEstimatedRange());

    // Filter to only plausible hands given the board and known cards
    vector<string> plausibleHands = filterPlausibleHands(allRanges, board, playerHand);

    // Handle edge case: no plausible hands
    if (plausibleHands.empty())
    {
        plausibleHands.push_back(ANY_CARDS_RANGE);
    }

    // Count how many of opponent's plausible hands beat us
    int nbWinningHands = 0;
    for (const string& hand : plausibleHands)
    {
        // Build hand string: "As Ah 2h 3h 4h" (need space between hole cards and board)
        std::string fullHand = hand;
        if (!board.empty())
        {
            fullHand += " " + board;
        }
        const int opponentHandRank = m_handEvaluator.rankHand(fullHand.c_str());
        if (opponentHandRank > playerHandRanking)
        {
            nbWinningHands++;
        }
    }

    // Log warning if original range was empty
    if (allRanges.empty())
    {
        m_logger.error("No ranges for opponent " + to_string(opponent.getId()));
        return 0.0f;
    }

    return static_cast<float>(nbWinningHands) / static_cast<float>(plausibleHands.size());
}

std::vector<std::string> OpponentsStrengthsEvaluator::filterPlausibleHands(const std::vector<std::string>& ranges,
                                                                           const std::string& board,
                                                                           const HoleCards& playerHand) const
{
    vector<string> plausibleHands;
    plausibleHands.reserve(ranges.size()); // Optimize memory allocation

    const string playerCard1Str = playerHand.card1.toString();
    const string playerCard2Str = playerHand.card2.toString();

    for (const string& handStr : ranges)
    {
        // Validate hand format
        if (handStr.size() != 4)
        {
            m_logger.error("Invalid hand format: " + handStr);
            continue;
        }

        const string card1 = handStr.substr(0, 2);
        const string card2 = handStr.substr(2, 2);

        // Remove hands with cards already on the board
        if (board.find(card1) != string::npos || board.find(card2) != string::npos)
        {
            continue;
        }

        // Remove hands with cards the player holds
        if (playerCard1Str.find(card1) != string::npos || playerCard2Str.find(card1) != string::npos ||
            playerCard1Str.find(card2) != string::npos || playerCard2Str.find(card2) != string::npos)
        {
            continue;
        }

        plausibleHands.push_back(handStr);
    }

    return plausibleHands;
}

} // namespace pkt::core::player
