/**
 * @file AllInShowdownTest.cpp
 * @brief Tests for all-in scenarios and showdown card dealing
 *
 * This test suite verifies that community cards are properly dealt when:
 * - A player goes all-in and is called by another player
 * - Multiple betting rounds occur with all-in situations
 * - The showdown reveals the correct community cards
 */

#include "common/DeterministicStrategy.h"
#include "common/EngineTest.h"
#include "core/player/Helpers.h"
#include <gtest/gtest.h>

using namespace pkt::core;
using namespace pkt::core::player;
using namespace pkt::test;

class AllInShowdownTest : public EngineTest
{
  protected:
    pkt::core::LogLevel getTestLogLevel() const override { return pkt::core::LogLevel::Verbose; }
};

/**
 * @test User goes all-in preflop, bot calls
 * Expected: All community cards (flop, turn, river) should be dealt automatically
 */
TEST_F(AllInShowdownTest, UserAllInPreflopBotCalls_ShouldDealAllCommunityCards)
{
    initializeHandWithPlayers(2, gameData);
    
    auto playerSb = getPlayerById(m_actingPlayersList, 0); // User (goes all-in)
    auto playerBb = getPlayerById(m_actingPlayersList, 1); // Bot (calls)
    
    // Set up strategies - SB goes all-in, BB calls
    auto sbStrategy = std::make_unique<DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Allin});
    playerSb->setStrategy(std::move(sbStrategy));
    
    auto bbStrategy = std::make_unique<DeterministicStrategy>();
    bbStrategy->setLastAction(GameState::Preflop, {playerBb->getId(), ActionType::Call});
    playerBb->setStrategy(std::move(bbStrategy));
    
    m_hand->runGameLoop();
    
    // Check that all 5 community cards have been dealt
    const auto& boardCards = m_hand->getBoard().getBoardCards();
    
    EXPECT_EQ(boardCards.numCards, 5) 
        << "All 5 community cards should be dealt when player goes all-in and is called";
    
    // Verify flop cards exist
    EXPECT_TRUE(boardCards.flop1.isValid());
    EXPECT_TRUE(boardCards.flop2.isValid());
    EXPECT_TRUE(boardCards.flop3.isValid());
    
    // Verify turn and river exist
    EXPECT_TRUE(boardCards.turn.isValid());
    EXPECT_TRUE(boardCards.river.isValid());
}

/**
 * @test User goes all-in on flop, bot calls
 * Expected: Turn and river should be dealt automatically
 */
TEST_F(AllInShowdownTest, UserAllInOnFlopBotCalls_ShouldDealTurnAndRiver)
{
    initializeHandWithPlayers(2, gameData);
    
    auto playerSb = getPlayerById(m_actingPlayersList, 0); // Button/SB (acts first preflop, second postflop)
    auto playerBb = getPlayerById(m_actingPlayersList, 1); // BB (acts second preflop, FIRST postflop in heads-up)
    
    // Set up strategies - both check/call preflop, then BB goes all-in on flop, SB calls
    auto sbStrategy = std::make_unique<DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Call});
    sbStrategy->setLastAction(GameState::Flop, {playerSb->getId(), ActionType::Call});  // SB calls BB's all-in
    playerSb->setStrategy(std::move(sbStrategy));
    
    auto bbStrategy = std::make_unique<DeterministicStrategy>();
    bbStrategy->setLastAction(GameState::Preflop, {playerBb->getId(), ActionType::Check});
    bbStrategy->setLastAction(GameState::Flop, {playerBb->getId(), ActionType::Allin});  // BB goes all-in first on flop
    playerBb->setStrategy(std::move(bbStrategy));
    
    m_hand->runGameLoop();
    
    // Turn and river should be dealt
    const auto& boardCards = m_hand->getBoard().getBoardCards();
    EXPECT_EQ(boardCards.numCards, 5) 
        << "Turn and river should be dealt when player goes all-in on flop and is called";
    
    EXPECT_TRUE(boardCards.turn.isValid());
    EXPECT_TRUE(boardCards.river.isValid());
}

/**
 * @test Bot goes all-in preflop, user calls
 * Expected: All community cards should be dealt automatically
 */
TEST_F(AllInShowdownTest, BotAllInPreflopUserCalls_ShouldDealAllCommunityCards)
{
    initializeHandWithPlayers(2, gameData);
    
    auto playerSb = getPlayerById(m_actingPlayersList, 0); // User (calls)
    auto playerBb = getPlayerById(m_actingPlayersList, 1); // Bot (goes all-in)
    
    // Reduce BB player's cash to force all-in scenario
    playerBb->setCash(50); // After posting BB (20), they'll have 30 left
    
    // Set up strategies - BB goes all-in, SB calls
    auto sbStrategy = std::make_unique<DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Call});
    playerSb->setStrategy(std::move(sbStrategy));
    
    auto bbStrategy = std::make_unique<DeterministicStrategy>();
    bbStrategy->setLastAction(GameState::Preflop, {playerBb->getId(), ActionType::Allin});
    playerBb->setStrategy(std::move(bbStrategy));
    
    m_hand->runGameLoop();
    
    // All community cards should be dealt
    const auto& boardCards = m_hand->getBoard().getBoardCards();
    
    EXPECT_EQ(boardCards.numCards, 5) 
        << "All 5 community cards should be dealt when bot goes all-in and user calls";
}

