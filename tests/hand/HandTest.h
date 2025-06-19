
#pragma once

#include <gtest/gtest.h>
#include <list>
#include <memory>
#include "core/engine/Board.h"
#include "core/engine/Hand.h"
#include "core/interfaces/NullLogger.h"
#include "core/player/Player.h"
#include "core/player/typedefs.h"

namespace pkt::test
{

class HandTest : public ::testing::Test
{
  protected:
    const pkt::core::GameEvents events;
    std::shared_ptr<pkt::core::EngineFactory> factory;
    std::shared_ptr<pkt::core::IHand> hand;
    std::shared_ptr<pkt::core::IBoard> board;
    std::shared_ptr<std::list<std::shared_ptr<pkt::core::player::Player>>> playerList;

    void SetUp() override;

    void TearDown() override;
    // Helper function to initialize a Hand object for testing
    void initializeHandForTesting(size_t activePlayerCount);

  private:
    // Helper function to create a PlayerList with mock players
    core::player::PlayerList createPlayerList(size_t playerCount);
    void initializeHandWithPlayers(core::player::PlayerList seatsList, size_t activePlayerCount);
};
} // namespace pkt::test