// HoldemCore — Tests for automatic fold when player has insufficient cash
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "common/EngineTest.h"
#include "common/DummyPlayer.h"
#include "core/player/Helpers.h"

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

class AutoFoldInsufficientCashTest : public EngineTest
{
  protected:
    pkt::core::LogLevel getTestLogLevel() const override { return pkt::core::LogLevel::Quiet; }
};

TEST_F(AutoFoldInsufficientCashTest, PlayersWithZeroCashNotDealtCards)
{
    // Arrange: Set up hand with players but don't initialize yet
    setupHandWithoutInitialize(4, gameData);
    
    // Set player 1 to have 0 cash BEFORE initialization
    auto p0 = getPlayerById(m_seatsList, 0);
    auto p1 = getPlayerById(m_seatsList, 1);
    auto p2 = getPlayerById(m_seatsList, 2);
    auto p3 = getPlayerById(m_seatsList, 3);
    
    p1->setCash(0);
    p1->setCashAtHandStart(0);
    
    // Act: Initialize (filters players) and deal cards
    m_hand->initialize();
    m_hand->dealHoleCards(0);  // Deal hole cards to remaining players
    
    // Assert: Player with 0 cash should not have cards
    EXPECT_FALSE(p1->getHoleCards().isValid()) << "Player with 0 cash should not receive hole cards";
    EXPECT_EQ(p1->getHandRanking(), 0) << "Player with 0 cash should have 0 hand ranking";
    
    // Verify other players got cards
    EXPECT_TRUE(p0->getHoleCards().isValid()) << "Player 0 with cash should receive hole cards";
    EXPECT_TRUE(p2->getHoleCards().isValid()) << "Player 2 with cash should receive hole cards";
    EXPECT_TRUE(p3->getHoleCards().isValid()) << "Player 3 with cash should receive hole cards";
}

TEST_F(AutoFoldInsufficientCashTest, AutoFoldedPlayersNotInActingList)
{
    // Arrange: Set up hand with players but don't initialize yet
    setupHandWithoutInitialize(6, gameData);
    
    // Set multiple players to have 0 cash BEFORE initialization
    auto p0 = getPlayerById(m_seatsList, 0);
    auto p1 = getPlayerById(m_seatsList, 1);
    auto p2 = getPlayerById(m_seatsList, 2);
    auto p3 = getPlayerById(m_seatsList, 3);
    auto p4 = getPlayerById(m_seatsList, 4);
    auto p5 = getPlayerById(m_seatsList, 5);
    
    // Only players 0 and 1 have cash
    p2->setCash(0);
    p2->setCashAtHandStart(0);
    p3->setCash(0);
    p3->setCashAtHandStart(0);
    p4->setCash(0);
    p4->setCashAtHandStart(0);
    p5->setCash(0);
    p5->setCashAtHandStart(0);
    
    // Act: Initialize hand
    m_hand->initialize();
    
    // Assert: Acting players list should only contain players 0 and 1
    auto actingPlayers = m_hand->getActingPlayersList();
    EXPECT_EQ(actingPlayers->size(), 2) << "Only 2 players with cash should be in acting list";

    // Verify the acting players are 0 and 1
    bool hasPlayer0 = false;
    bool hasPlayer1 = false;
    for (const auto& player : *actingPlayers)
    {
        if (player->getId() == 0) hasPlayer0 = true;
        if (player->getId() == 1) hasPlayer1 = true;
    }
    EXPECT_TRUE(hasPlayer0) << "Player 0 should be in acting list";
    EXPECT_TRUE(hasPlayer1) << "Player 1 should be in acting list";
}

} // namespace pkt::test