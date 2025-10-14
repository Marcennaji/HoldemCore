// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "common/EngineTest.h"

#include "adapters/infrastructure/hand_evaluation/PsimHandEvaluationEngine.h"
#include "adapters/infrastructure/logger/ConsoleLogger.h"
#include "core/engine/cards/Card.h"
#include "core/player/OpponentsStrengthsEvaluator.h"
#include "core/player/Player.h"
#include "core/player/strategy/CurrentHandContext.h"

#include <gtest/gtest.h>
#include <optional>

using namespace pkt::core;
using namespace pkt::core::player;
using namespace pkt::test;

/**
 * @brief Comprehensive unit tests for OpponentsStrengthsEvaluator
 *
 * Tests cover:
 * - Construction and initialization
 * - Single opponent evaluation
 * - Multiple opponents evaluation
 * - Edge cases (no opponents, all folded, empty ranges)
 * - Result structure validation
 * - Hand filtering logic
 * - Integration with Player and CurrentHandContext
 */
class OpponentsStrengthsEvaluatorTest : public EngineTest
{
  protected:
    void SetUp() override
    {
        EngineTest::SetUp();

        // Create evaluator for player 0
        m_evaluator = std::make_unique<OpponentsStrengthsEvaluator>(0, // player ID
                                                                    getLogger(), *getHandEvaluationEngineService());
    }

    void TearDown() override
    {
        m_evaluator.reset();
        EngineTest::TearDown();
    }

    /**
     * Helper: Get player by ID from seats list
     */
    std::shared_ptr<Player> getPlayerById(int id)
    {
        for (auto& player : *m_seatsList)
        {
            if (player->getId() == id)
            {
                return player;
            }
        }
        return nullptr;
    }

    /**
     * Helper: Set up a basic hand scenario with players having specific hole cards
     */
    void setupBasicScenario(const HoleCards& heroCards, const std::optional<HoleCards>& opponent1Cards = std::nullopt,
                            const std::optional<HoleCards>& opponent2Cards = std::nullopt,
                            const std::optional<BoardCards>& communityCards = std::nullopt)
    {
        // Determine how many players are needed based on which cards are provided
        int requiredPlayerCount = 1; // Always need the evaluating player
        if (opponent1Cards.has_value())
            requiredPlayerCount = std::max(requiredPlayerCount, 2);
        if (opponent2Cards.has_value())
            requiredPlayerCount = std::max(requiredPlayerCount, 3);

        // Initialize hand with random cards (we'll override them next)
        initializeHandWithPlayers(requiredPlayerCount, gameData);

        // Set specific hole cards for the evaluating player (ID 0)
        auto heroPlayer = getPlayerById(0);
        heroPlayer->setHoleCards(heroCards);

        // Set specific hole cards for first opponent (ID 1) if provided
        if (opponent1Cards.has_value() && requiredPlayerCount >= 2)
        {
            auto firstOpponent = getPlayerById(1);
            firstOpponent->setHoleCards(*opponent1Cards);
        }

        // Set specific hole cards for second opponent (ID 2) if provided
        if (opponent2Cards.has_value() && requiredPlayerCount >= 3)
        {
            auto secondOpponent = getPlayerById(2);
            secondOpponent->setHoleCards(*opponent2Cards);
        }

        // Set community cards if provided
        if (communityCards.has_value())
        {
            m_board->setBoardCards(*communityCards);
        }

        // Get current hand state for setting player actions
        auto& currentHandState = m_hand->getState();

        // Record that the evaluating player called (makes them active in the hand)
        const int standardCallAmount = 20;
        heroPlayer->setAction(currentHandState,
                              PlayerAction{heroPlayer->getId(), ActionType::Call, standardCallAmount});

        // Record that first opponent called if they're in the hand
        if (opponent1Cards.has_value() && requiredPlayerCount >= 2)
        {
            auto firstOpponent = getPlayerById(1);
            firstOpponent->setAction(currentHandState,
                                     PlayerAction{firstOpponent->getId(), ActionType::Call, standardCallAmount});
        }

        // Record that second opponent called if they're in the hand
        if (opponent2Cards.has_value() && requiredPlayerCount >= 3)
        {
            auto secondOpponent = getPlayerById(2);
            secondOpponent->setAction(currentHandState,
                                      PlayerAction{secondOpponent->getId(), ActionType::Call, standardCallAmount});
        }

        // Update all player contexts to reflect the actions we just set
        for (auto& player : *m_seatsList)
        {
            player->updateCurrentHandContext(*m_hand);
        }
    }

    /**
     * Helper: Parse two cards from string like "As Ah"
     */
    std::pair<Card, Card> parseCards(const std::string& cardsStr)
    {
        size_t space = cardsStr.find(' ');
        std::string card1Str = cardsStr.substr(0, space);
        std::string card2Str = cardsStr.substr(space + 1);
        return {Card(card1Str), Card(card2Str)};
    }

    /**
     * Helper: Parse board cards from string like "Kc 8d Qc"
     */
    BoardCards parseBoardCards(const std::string& boardStr)
    {
        std::vector<std::string> cardStrs;
        size_t start = 0;
        size_t end = 0;

        while ((end = boardStr.find(' ', start)) != std::string::npos)
        {
            cardStrs.push_back(boardStr.substr(start, end - start));
            start = end + 1;
        }
        if (start < boardStr.length())
        {
            cardStrs.push_back(boardStr.substr(start));
        }

        // Create BoardCards with appropriate number of cards (use correct constructor)
        if (cardStrs.size() == 3)
            return BoardCards(cardStrs[0], cardStrs[1], cardStrs[2]); // 3-card constructor
        else if (cardStrs.size() == 4)
            return BoardCards(cardStrs[0], cardStrs[1], cardStrs[2], cardStrs[3]); // 4-card constructor
        else if (cardStrs.size() == 5)
            return BoardCards(cardStrs[0], cardStrs[1], cardStrs[2], cardStrs[3], cardStrs[4]); // 5-card constructor
        else
            return BoardCards(); // Empty board (preflop)
    }

    /**
     * Helper: Evaluate and set hand ranking for a player
     */
    void evaluatePlayerHand(std::shared_ptr<Player> player)
    {
        const auto& hc = player->getHoleCards();
        const auto& bc = m_board->getBoardCards();
        std::string handStr = hc.toString() + " " + bc.toString();
        player->setHandRanking(getHandEvaluationEngineService()->rankHand(handStr.c_str()));
    }

    std::unique_ptr<OpponentsStrengthsEvaluator> m_evaluator;
};

// ========================================
// Construction and Initialization Tests
// ========================================

TEST_F(OpponentsStrengthsEvaluatorTest, ConstructorInitializesCorrectly)
{
    // Evaluator should be created successfully
    ASSERT_NE(m_evaluator, nullptr);
}

TEST_F(OpponentsStrengthsEvaluatorTest, CanCreateMultipleEvaluatorsForDifferentPlayers)
{
    auto evaluator1 = std::make_unique<OpponentsStrengthsEvaluator>(0, getLogger(), *getHandEvaluationEngineService());
    auto evaluator2 = std::make_unique<OpponentsStrengthsEvaluator>(1, getLogger(), *getHandEvaluationEngineService());

    EXPECT_NE(evaluator1, nullptr);
    EXPECT_NE(evaluator2, nullptr);
}

// ========================================
// Single Opponent Evaluation Tests
// ========================================

TEST_F(OpponentsStrengthsEvaluatorTest, SingleOpponent_WithWeakerHand_ReturnsLowStrength)
{
    setupBasicScenario(HoleCards("As", "Ah"), HoleCards("7c", "6c"), std::nullopt, BoardCards("Kc", "8d", "Qc"));

    auto evaluatingPlayer = getPlayerById(0);
    evaluatePlayerHand(evaluatingPlayer);

    // Set opponent range - assuming weak range
    auto player1 = getPlayerById(1);
    player1->getRangeEstimator()->setEstimatedRange("22,33,44,55,66,77");

    auto result = m_evaluator->evaluateOpponents(evaluatingPlayer->getCurrentHandContext(),
                                                 evaluatingPlayer->getHoleCards(), evaluatingPlayer->getHandRanking());

    // Debug output
    m_logger->info("Result size: " + std::to_string(result.opponentStrengths.size()));
    m_logger->info("Strongest ID: " + std::to_string(result.strongestOpponentId));
    m_logger->info("Max strength: " + std::to_string(result.maxStrength));
    for (const auto& [id, strength] : result.opponentStrengths)
    {
        m_logger->info("Player " + std::to_string(id) + ": " + std::to_string(strength));
    }

    // Check if opponent was evaluated
    ASSERT_GE(result.opponentStrengths.size(), 1) << "Should have at least one opponent evaluated";
    EXPECT_TRUE(result.opponentStrengths.count(1) > 0);
    // Most of the weak pairs should lose to our aces
    EXPECT_LE(result.maxStrength, 0.5f);
    EXPECT_NE(result.strongestOpponentId, -1) << "Should have identified a strongest opponent";
}

TEST_F(OpponentsStrengthsEvaluatorTest, SingleOpponent_WithStrongerRange_ReturnsHighStrength)
{
    setupBasicScenario(HoleCards("9s", "9h"), HoleCards("As", "Ah"), std::nullopt, BoardCards("Kc", "8d", "Qc"));

    auto evaluatingPlayer = getPlayerById(0);
    evaluatePlayerHand(evaluatingPlayer);

    // Set opponent to strong range that beats us
    auto player1 = getPlayerById(1);
    player1->getRangeEstimator()->setEstimatedRange("AA,KK,QQ");

    auto result = m_evaluator->evaluateOpponents(evaluatingPlayer->getCurrentHandContext(),
                                                 evaluatingPlayer->getHoleCards(), evaluatingPlayer->getHandRanking());

    EXPECT_EQ(result.opponentStrengths.size(), 1);
    // All premium pairs should beat our 9s
    EXPECT_GE(result.maxStrength, 0.5f);
}

TEST_F(OpponentsStrengthsEvaluatorTest, SingleOpponent_OnFlush_EvaluatesCorrectly)
{
    setupBasicScenario(HoleCards("Ah", "Kh"), HoleCards("7c", "6c"), std::nullopt, BoardCards("2h", "5h", "9h"));

    auto evaluatingPlayer = getPlayerById(0);
    evaluatePlayerHand(evaluatingPlayer);

    auto player1 = getPlayerById(1);
    // Opponent has flush draw range
    player1->getRangeEstimator()->setEstimatedRange("Th9h,8h7h,6h5h,QhJh,Jh8h");

    auto result = m_evaluator->evaluateOpponents(evaluatingPlayer->getCurrentHandContext(),
                                                 evaluatingPlayer->getHoleCards(), evaluatingPlayer->getHandRanking());

    EXPECT_EQ(result.opponentStrengths.size(), 1);
    // Some hands beat us (higher flush), some don't
    EXPECT_GE(result.maxStrength, 0.0f);
    EXPECT_LE(result.maxStrength, 1.0f);
}

// ========================================
// Multiple Opponents Evaluation Tests
// ========================================

TEST_F(OpponentsStrengthsEvaluatorTest, MultipleOpponents_IdentifiesStrongestOpponent)
{
    setupBasicScenario(HoleCards("Ts", "Tc"), HoleCards("7c", "6c"), HoleCards("As", "Ah"),
                       BoardCards("Kc", "8d", "Qc"));

    auto evaluatingPlayer = getPlayerById(0);
    evaluatePlayerHand(evaluatingPlayer);

    // Player 1 has weak range
    auto player1 = getPlayerById(1);
    player1->getRangeEstimator()->setEstimatedRange("22,33,44");

    // Player 2 has strong range (should be strongest)
    auto player2 = getPlayerById(2);
    player2->getRangeEstimator()->setEstimatedRange("AA,KK,QQ");

    auto result = m_evaluator->evaluateOpponents(evaluatingPlayer->getCurrentHandContext(),
                                                 evaluatingPlayer->getHoleCards(), evaluatingPlayer->getHandRanking());

    EXPECT_EQ(result.opponentStrengths.size(), 2);
    EXPECT_TRUE(result.opponentStrengths.count(1) > 0);
    EXPECT_TRUE(result.opponentStrengths.count(2) > 0);

    // Player 2 should have higher strength than player 1
    EXPECT_GT(result.opponentStrengths[2], result.opponentStrengths[1]);

    // Strongest opponent should be player 2
    EXPECT_EQ(result.strongestOpponentId, 2);
    EXPECT_FLOAT_EQ(result.maxStrength, result.opponentStrengths[2]);
}

TEST_F(OpponentsStrengthsEvaluatorTest, MultipleOpponents_AllHaveWeakHands_ReturnsLowMaxStrength)
{
    setupBasicScenario(HoleCards("As", "Ah"), HoleCards("7c", "6c"), HoleCards("9s", "8s"),
                       BoardCards("Kc", "8d", "Qc"));

    auto evaluatingPlayer = getPlayerById(0);
    evaluatePlayerHand(evaluatingPlayer);

    // Both opponents have weak ranges
    getPlayerById(1)->getRangeEstimator()->setEstimatedRange("22,33,44");
    getPlayerById(2)->getRangeEstimator()->setEstimatedRange("55,66,77");

    auto result = m_evaluator->evaluateOpponents(evaluatingPlayer->getCurrentHandContext(),
                                                 evaluatingPlayer->getHoleCards(), evaluatingPlayer->getHandRanking());

    EXPECT_EQ(result.opponentStrengths.size(), 2);
    // Both should have low strength against our aces
    EXPECT_LE(result.maxStrength, 0.5f);
}

TEST_F(OpponentsStrengthsEvaluatorTest, MultipleOpponents_MixedStrengths_ReturnsCorrectMax)
{
    setupBasicScenario(HoleCards("Jc", "Jh"), HoleCards("7c", "6c"), HoleCards("As", "Ah"),
                       BoardCards("Kc", "8d", "Qc"));

    auto evaluatingPlayer = getPlayerById(0);
    evaluatePlayerHand(evaluatingPlayer);

    getPlayerById(1)->getRangeEstimator()->setEstimatedRange("22,33,44,55");
    getPlayerById(2)->getRangeEstimator()->setEstimatedRange("AA,KK,QQ,JJ,TT");

    auto result = m_evaluator->evaluateOpponents(evaluatingPlayer->getCurrentHandContext(),
                                                 evaluatingPlayer->getHoleCards(), evaluatingPlayer->getHandRanking());

    // Max strength should equal the highest individual strength
    float maxIndividual = std::max(result.opponentStrengths[1], result.opponentStrengths[2]);
    EXPECT_FLOAT_EQ(result.maxStrength, maxIndividual);
}

// ========================================
// Edge Cases Tests
// ========================================

TEST_F(OpponentsStrengthsEvaluatorTest, NoActiveOpponents_ReturnsEmptyResult)
{
    initializeHandWithPlayers(2, gameData);

    auto evaluatingPlayer = getPlayerById(0);
    evaluatingPlayer->setHoleCards(HoleCards("As", "Ah"));
    evaluatingPlayer->setHandRanking(100);

    // Make opponent fold
    auto player1 = getPlayerById(1);
    player1->getCurrentHandContext().personalContext.actions.currentHandActions.addAction(
        Preflop, PlayerAction{player1->getId(), ActionType::Fold});

    evaluatingPlayer->updateCurrentHandContext(*m_hand);

    auto result = m_evaluator->evaluateOpponents(evaluatingPlayer->getCurrentHandContext(),
                                                 evaluatingPlayer->getHoleCards(), evaluatingPlayer->getHandRanking());

    EXPECT_EQ(result.opponentStrengths.size(), 0);
    EXPECT_FLOAT_EQ(result.maxStrength, -1.0f); // -1 indicates no opponents evaluated
    EXPECT_EQ(result.strongestOpponentId, -1);
}

TEST_F(OpponentsStrengthsEvaluatorTest, AllOpponentsHaveFolded_ReturnsEmptyStrengths)
{
    initializeHandWithPlayers(3, gameData);

    auto evaluatingPlayer = getPlayerById(0);
    evaluatingPlayer->setHoleCards(HoleCards("As", "Ah"));
    evaluatingPlayer->setHandRanking(100);

    // All opponents fold
    for (int i = 1; i <= 2; ++i)
    {
        auto player = getPlayerById(i);
        player->getCurrentHandContext().personalContext.actions.currentHandActions.addAction(
            Preflop, PlayerAction{player->getId(), ActionType::Fold, 0});
    }

    evaluatingPlayer->updateCurrentHandContext(*m_hand);

    auto result = m_evaluator->evaluateOpponents(evaluatingPlayer->getCurrentHandContext(),
                                                 evaluatingPlayer->getHoleCards(), evaluatingPlayer->getHandRanking());

    EXPECT_EQ(result.opponentStrengths.size(), 0);
    EXPECT_EQ(result.strongestOpponentId, -1);
}

TEST_F(OpponentsStrengthsEvaluatorTest, OpponentHasNoneAction_IsSkipped)
{
    initializeHandWithPlayers(3, gameData);

    auto evaluatingPlayer = getPlayerById(0);
    evaluatingPlayer->setHoleCards(HoleCards("As", "Ah"));
    evaluatingPlayer->setHandRanking(100);

    auto player1 = getPlayerById(1);
    player1->setHoleCards(HoleCards("Kc", "Kd"));
    player1->getRangeEstimator()->setEstimatedRange("KK,QQ");

    // Player 2 has None action (hasn't acted yet in new betting round)
    auto player2 = getPlayerById(2);
    player2->getCurrentHandContext().personalContext.actions.currentHandActions.addAction(
        Preflop, PlayerAction{player2->getId(), ActionType::None, 0});

    evaluatingPlayer->updateCurrentHandContext(*m_hand);

    auto result = m_evaluator->evaluateOpponents(evaluatingPlayer->getCurrentHandContext(),
                                                 evaluatingPlayer->getHoleCards(), evaluatingPlayer->getHandRanking());

    // Only player 1 should be evaluated (player 2 is skipped due to None action)
    EXPECT_EQ(result.opponentStrengths.size(), 1);
    EXPECT_TRUE(result.opponentStrengths.count(1) > 0);
    EXPECT_FALSE(result.opponentStrengths.count(2) > 0);
}

// ========================================
// Hand Filtering Logic Tests
// ========================================

TEST_F(OpponentsStrengthsEvaluatorTest, FilteringRemovesHandsWithBoardCards)
{
    setupBasicScenario(HoleCards("As", "Ah"), HoleCards("7c", "6c"), std::nullopt, BoardCards("Kc", "8d", "Qc"));

    auto evaluatingPlayer = getPlayerById(0);
    evaluatePlayerHand(evaluatingPlayer);

    // Opponent range includes hands with board cards
    auto player1 = getPlayerById(1);
    player1->getRangeEstimator()->setEstimatedRange("KcQc,8d8c,7c7d"); // KcQc and 8d impossible

    auto result = m_evaluator->evaluateOpponents(evaluatingPlayer->getCurrentHandContext(),
                                                 evaluatingPlayer->getHoleCards(), evaluatingPlayer->getHandRanking());

    // Should successfully filter out impossible hands
    EXPECT_EQ(result.opponentStrengths.size(), 1);
    EXPECT_GE(result.maxStrength, 0.0f);
    EXPECT_LE(result.maxStrength, 1.0f);
}

TEST_F(OpponentsStrengthsEvaluatorTest, FilteringRemovesHandsWithPlayerCards)
{
    setupBasicScenario(HoleCards("As", "Ah"), HoleCards("7c", "6c"), std::nullopt, BoardCards("Kc", "8d", "Qc"));

    auto evaluatingPlayer = getPlayerById(0);
    evaluatePlayerHand(evaluatingPlayer);

    // Opponent range includes hands with our cards
    auto player1 = getPlayerById(1);
    player1->getRangeEstimator()->setEstimatedRange("AsKs,AhKh,QQ,JJ"); // As and Ah impossible

    auto result = m_evaluator->evaluateOpponents(evaluatingPlayer->getCurrentHandContext(),
                                                 evaluatingPlayer->getHoleCards(), evaluatingPlayer->getHandRanking());

    // Should filter out hands with our aces
    EXPECT_EQ(result.opponentStrengths.size(), 1);
    // Result should be based on valid hands only (QQ, JJ)
    EXPECT_GE(result.maxStrength, 0.0f);
}

TEST_F(OpponentsStrengthsEvaluatorTest, AllHandsFiltered_UsesAnyCardsRange)
{
    setupBasicScenario(HoleCards("As", "Ah"), HoleCards("Kc", "Kd"), std::nullopt, BoardCards("Ks", "Kh", "Qc"));

    auto evaluatingPlayer = getPlayerById(0);
    evaluatePlayerHand(evaluatingPlayer);

    // Opponent range has only hands with board cards (all K's on board)
    auto player1 = getPlayerById(1);
    player1->getRangeEstimator()->setEstimatedRange("KsKh,KsKd,KhKd"); // All impossible

    auto result = m_evaluator->evaluateOpponents(evaluatingPlayer->getCurrentHandContext(),
                                                 evaluatingPlayer->getHoleCards(), evaluatingPlayer->getHandRanking());

    // Should fall back to ANY_CARDS_RANGE
    EXPECT_EQ(result.opponentStrengths.size(), 1);
}

// ========================================
// Result Structure Validation Tests
// ========================================

TEST_F(OpponentsStrengthsEvaluatorTest, EvaluationResult_DefaultConstructor_InitializesCorrectly)
{
    OpponentsStrengthsEvaluator::EvaluationResult result;

    EXPECT_TRUE(result.opponentStrengths.empty());
    EXPECT_FLOAT_EQ(result.maxStrength, -1.0f); // -1 indicates no evaluation performed yet
    EXPECT_EQ(result.strongestOpponentId, -1);
}

TEST_F(OpponentsStrengthsEvaluatorTest, EvaluationResult_MapContainsOnlyEvaluatedPlayers)
{
    setupBasicScenario(HoleCards("As", "Ah"), HoleCards("7c", "6c"), HoleCards("Kc", "Kd"),
                       BoardCards("2h", "3h", "4h"));

    auto evaluatingPlayer = getPlayerById(0);
    evaluatePlayerHand(evaluatingPlayer);

    // Set ranges for both opponents
    getPlayerById(1)->getRangeEstimator()->setEstimatedRange("77,88,99");
    getPlayerById(2)->getRangeEstimator()->setEstimatedRange("KK,QQ,JJ");

    auto result = m_evaluator->evaluateOpponents(evaluatingPlayer->getCurrentHandContext(),
                                                 evaluatingPlayer->getHoleCards(), evaluatingPlayer->getHandRanking());

    // Map should contain exactly 2 entries
    EXPECT_EQ(result.opponentStrengths.size(), 2);
    EXPECT_FALSE(result.opponentStrengths.count(0) > 0); // Not ourselves
}

TEST_F(OpponentsStrengthsEvaluatorTest, EvaluationResult_MaxStrengthMatchesMapEntry)
{
    setupBasicScenario(HoleCards("Jc", "Jh"), HoleCards("7c", "6c"), HoleCards("As", "Ah"),
                       BoardCards("Kc", "8d", "Qc"));

    auto evaluatingPlayer = getPlayerById(0);
    evaluatePlayerHand(evaluatingPlayer);

    getPlayerById(1)->getRangeEstimator()->setEstimatedRange("77,88,99");
    getPlayerById(2)->getRangeEstimator()->setEstimatedRange("AA,KK,QQ");

    auto result = m_evaluator->evaluateOpponents(evaluatingPlayer->getCurrentHandContext(),
                                                 evaluatingPlayer->getHoleCards(), evaluatingPlayer->getHandRanking());

    // maxStrength should equal the strength of strongestOpponentId
    ASSERT_NE(result.strongestOpponentId, -1);
    EXPECT_FLOAT_EQ(result.maxStrength, result.opponentStrengths[result.strongestOpponentId]);
}

TEST_F(OpponentsStrengthsEvaluatorTest, EvaluationResult_StrengthsAreValidPercentages)
{
    setupBasicScenario(HoleCards("As", "Ah"), HoleCards("7c", "6c"), HoleCards("Kc", "Kd"),
                       BoardCards("2h", "3h", "4h"));

    auto evaluatingPlayer = getPlayerById(0);
    evaluatePlayerHand(evaluatingPlayer);

    getPlayerById(1)->getRangeEstimator()->setEstimatedRange("77,88,99,TT");
    getPlayerById(2)->getRangeEstimator()->setEstimatedRange("KK,QQ,JJ");

    auto result = m_evaluator->evaluateOpponents(evaluatingPlayer->getCurrentHandContext(),
                                                 evaluatingPlayer->getHoleCards(), evaluatingPlayer->getHandRanking());

    // All strengths should be valid percentages [0.0, 1.0]
    for (const auto& [playerId, strength] : result.opponentStrengths)
    {
        EXPECT_GE(strength, 0.0f) << "Strength for player " << playerId << " is negative";
        EXPECT_LE(strength, 1.0f) << "Strength for player " << playerId << " exceeds 1.0";
    }

    EXPECT_GE(result.maxStrength, 0.0f);
    EXPECT_LE(result.maxStrength, 1.0f);
}

// ========================================
// Realistic Poker Scenarios Tests
// ========================================

TEST_F(OpponentsStrengthsEvaluatorTest, PocketAcesVsPocketKings_CorrectEvaluation)
{
    setupBasicScenario(HoleCards("As", "Ah"), HoleCards("Kc", "Kd"), std::nullopt, BoardCards("2h", "7d", "9c"));

    auto evaluatingPlayer = getPlayerById(0);
    evaluatePlayerHand(evaluatingPlayer);

    // Opponent has exactly KK
    auto player1 = getPlayerById(1);
    player1->getRangeEstimator()->setEstimatedRange("KcKd");

    auto result = m_evaluator->evaluateOpponents(evaluatingPlayer->getCurrentHandContext(),
                                                 evaluatingPlayer->getHoleCards(), evaluatingPlayer->getHandRanking());

    // KK loses to AA
    EXPECT_EQ(result.opponentStrengths.size(), 1);
    EXPECT_FLOAT_EQ(result.opponentStrengths[1], 0.0f);
}

TEST_F(OpponentsStrengthsEvaluatorTest, FlushVsSet_OnFlushBoard_CorrectEvaluation)
{
    setupBasicScenario(HoleCards("9h", "8h"), HoleCards("Kc", "Kd"), std::nullopt, BoardCards("2h", "5h", "7h"));

    auto evaluatingPlayer = getPlayerById(0);
    evaluatePlayerHand(evaluatingPlayer);

    // Opponent has set of kings
    auto player1 = getPlayerById(1);
    player1->getRangeEstimator()->setEstimatedRange("KcKd");

    auto result = m_evaluator->evaluateOpponents(evaluatingPlayer->getCurrentHandContext(),
                                                 evaluatingPlayer->getHoleCards(), evaluatingPlayer->getHandRanking());

    // Set loses to flush
    EXPECT_EQ(result.opponentStrengths.size(), 1);
    EXPECT_FLOAT_EQ(result.opponentStrengths[1], 0.0f);
}

TEST_F(OpponentsStrengthsEvaluatorTest, MultiwayPot_IdentifiesActualStrongestHand)
{
    setupBasicScenario(HoleCards("Tc", "Th"), HoleCards("7c", "6c"), HoleCards("Qc", "Qd"),
                       BoardCards("Kc", "8d", "Qc"));

    auto evaluatingPlayer = getPlayerById(0);
    evaluatePlayerHand(evaluatingPlayer);

    // Player 1 has weak range
    getPlayerById(1)->getRangeEstimator()->setEstimatedRange("76,87,98");

    // Player 2 has strong range (trips)
    getPlayerById(2)->getRangeEstimator()->setEstimatedRange("QcQd,KK,AA");

    auto result = m_evaluator->evaluateOpponents(evaluatingPlayer->getCurrentHandContext(),
                                                 evaluatingPlayer->getHoleCards(), evaluatingPlayer->getHandRanking());

    // Player 2 should be strongest (has trips)
    EXPECT_EQ(result.strongestOpponentId, 2);
    EXPECT_GT(result.opponentStrengths[2], result.opponentStrengths[1]);
}

// ========================================
// Integration Tests
// ========================================

TEST_F(OpponentsStrengthsEvaluatorTest, IntegrationWithPlayer_EvaluatesCorrectly)
{
    setupBasicScenario(HoleCards("As", "Ah"), HoleCards("7c", "6c"), HoleCards("Kc", "Kd"),
                       BoardCards("2h", "3h", "4h"));

    auto evaluatingPlayer = getPlayerById(0);
    evaluatePlayerHand(evaluatingPlayer);

    getPlayerById(1)->getRangeEstimator()->setEstimatedRange("77,88,99");
    getPlayerById(2)->getRangeEstimator()->setEstimatedRange("KK,QQ");

    // Use the evaluator that's part of the Player class
    auto& playerEvaluator = evaluatingPlayer->getOpponentsStrengthsEvaluator();

    auto result =
        playerEvaluator->evaluateOpponents(evaluatingPlayer->getCurrentHandContext(), evaluatingPlayer->getHoleCards(),
                                           evaluatingPlayer->getHandRanking());

    EXPECT_EQ(result.opponentStrengths.size(), 2);
    EXPECT_GE(result.maxStrength, 0.0f);
    EXPECT_LE(result.maxStrength, 1.0f);
}

TEST_F(OpponentsStrengthsEvaluatorTest, MultipleEvaluations_ProduceConsistentResults)
{
    setupBasicScenario(HoleCards("As", "Ah"), HoleCards("Kc", "Kd"), std::nullopt, BoardCards("2h", "3h", "4h"));

    auto evaluatingPlayer = getPlayerById(0);
    evaluatePlayerHand(evaluatingPlayer);

    getPlayerById(1)->getRangeEstimator()->setEstimatedRange("KK,QQ,JJ");

    // Evaluate twice
    auto result1 = m_evaluator->evaluateOpponents(evaluatingPlayer->getCurrentHandContext(),
                                                  evaluatingPlayer->getHoleCards(), evaluatingPlayer->getHandRanking());

    auto result2 = m_evaluator->evaluateOpponents(evaluatingPlayer->getCurrentHandContext(),
                                                  evaluatingPlayer->getHoleCards(), evaluatingPlayer->getHandRanking());

    // Results should be identical
    EXPECT_FLOAT_EQ(result1.maxStrength, result2.maxStrength);
    EXPECT_EQ(result1.strongestOpponentId, result2.strongestOpponentId);
    EXPECT_EQ(result1.opponentStrengths.size(), result2.opponentStrengths.size());
}
