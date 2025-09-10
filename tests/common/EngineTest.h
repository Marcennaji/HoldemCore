
#pragma once

#include "common/common.h"

#include "core/engine/BoardFsm.h"
#include "core/engine/deprecated/Board.h"
#include "core/engine/deprecated/Hand.h"
#include "core/engine/model/GameState.h"
#include "core/interfaces/NullLogger.h"
#include "core/player/deprecated/Player.h"
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

    void initializeHandFsmWithPlayers(size_t activePlayerCount, pkt::core::GameData);
    void initializeHandWithPlayers(size_t activePlayerCount, pkt::core::GameData);
    void checkPostRiverConditions();
    bool isPlayerStillActive(unsigned id) const;

    pkt::core::GameEvents myEvents;
    std::shared_ptr<pkt::core::EngineFactory> myFactory;
    std::shared_ptr<pkt::core::IHand> myHand;
    std::shared_ptr<pkt::core::HandFsm> myHandFsm;
    std::shared_ptr<pkt::core::IBoard> myBoard;
    std::shared_ptr<pkt::core::IBoard> myBoardFsm;
    std::shared_ptr<std::list<std::shared_ptr<pkt::core::player::Player>>> mySeatsList;
    std::shared_ptr<std::list<std::shared_ptr<pkt::core::player::Player>>> myActingPlayersList;

    std::shared_ptr<std::list<std::shared_ptr<pkt::core::player::PlayerFsm>>> mySeatsListFsm;
    std::shared_ptr<std::list<std::shared_ptr<pkt::core::player::PlayerFsm>>> myActingPlayersListFsm;
    pkt::core::GameState myLastGameState = pkt::core::None;

    pkt::core::GameData gameData;

  private:
    // Helper function to create a PlayerList with mock players
    void createPlayersLists(size_t playerCount);
    void createPlayersFsmLists(size_t playerCount);

    const int startDealerPlayerId{0}; // Starting dealer player ID, can be set to any valid player ID
};
} // namespace pkt::test