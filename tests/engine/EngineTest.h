
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

class EngineTest : public ::testing::Test
{
  protected:
    pkt::core::GameEvents myEvents;
    std::shared_ptr<pkt::core::EngineFactory> myFactory;
    std::shared_ptr<pkt::core::IHand> myHand;
    std::shared_ptr<pkt::core::IBoard> myBoard;
    std::shared_ptr<std::list<std::shared_ptr<pkt::core::player::Player>>> myPlayerList;

    void SetUp() override;

    void TearDown() override;
    // Helper function to initialize a Hand object for testing
    void initializeHandForTesting(size_t activePlayerCount);

  private:
    // Helper function to create a PlayerList with mock players
    void createPlayerList(size_t playerCount);
    void initializeHandWithPlayers(size_t activePlayerCount);

    static const int startDealerPlayerId = 1;
};
} // namespace pkt::test