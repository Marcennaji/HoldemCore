
#pragma once

#include "common/common.h"

#include "core/engine/EngineFactory.h"
#include "core/engine/game/Board.h"
#include "core/engine/model/GameData.h"
#include "core/engine/model/GameState.h"
#include "core/interfaces/NullLogger.h"
#include "core/player/Player.h"
#include "core/player/typedefs.h"

#include <list>
#include <memory>

namespace pkt::test
{

class EngineTest : public ::testing::Test
{
  protected:
    void SetUp() override;
    void TearDown() override;

    void initializeHandWithPlayers(size_t activePlayerCount, pkt::core::GameData);
    void checkPostRiverConditions();
    bool isPlayerStillActive(unsigned id) const;

    // Access to services for test logging
    pkt::core::Logger& getLogger() const;
    std::shared_ptr<pkt::core::ServiceContainer> getServices() const;

    pkt::core::GameEvents myEvents;
    std::shared_ptr<pkt::core::EngineFactory> myFactory;
    std::shared_ptr<pkt::core::Hand> myHand;
    std::shared_ptr<pkt::core::Board> myBoard;

    std::shared_ptr<std::list<std::shared_ptr<pkt::core::player::Player>>> mySeatsList;
    std::shared_ptr<std::list<std::shared_ptr<pkt::core::player::Player>>> myActingPlayersList;
    pkt::core::GameState myLastGameState = pkt::core::None;

    pkt::core::GameData gameData;

  private:
    void createPlayersLists(size_t playerCount);

    std::shared_ptr<pkt::core::AppServiceContainer> myServices; // Store services for logger access
    const int startDealerPlayerId{0}; // Starting dealer player ID, can be set to any valid player ID
};
} // namespace pkt::test