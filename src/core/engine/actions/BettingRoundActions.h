#pragma once

#include <optional>
#include "core/engine/model/GameState.h"
#include "core/engine/model/PlayerPosition.h"
#include "core/engine/hand/HandState.h"
#include "core/player/typedefs.h"

#include <vector>
#include <memory>

namespace pkt::core
{
class BettingRoundActions
{
  public:
    BettingRoundActions(GameState gameState, pkt::core::player::PlayerList& seats,
                        pkt::core::player::PlayerList& actingPlayers);

    int getCallsNumber();
    int getRaisesNumber();
    int getBetsOrRaisesNumber();

    std::vector<PlayerPosition> getRaisersPositions();
    std::vector<PlayerPosition> getCallersPositions();

    std::shared_ptr<pkt::core::player::Player> getLastRaiser();
    void setLastRaiser(std::shared_ptr<pkt::core::player::Player> player);

  protected:
    GameState m_gameState;
    std::shared_ptr<pkt::core::player::Player> m_lastRaiser = nullptr;

    const pkt::core::player::PlayerList& m_seatsList;
    const pkt::core::player::PlayerList& m_actingPlayersList;
};

} // namespace pkt::core
