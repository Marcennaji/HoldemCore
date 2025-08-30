#pragma once

#include <optional>
#include "core/engine/model/GameState.h"
#include "core/engine/model/PlayerPosition.h"
#include "core/interfaces/hand/IHandState.h"
#include "core/player/typedefs.h"

#include <vector>

namespace pkt::core
{
class BettingRoundActions
{
  public:
    BettingRoundActions(GameState gameState, pkt::core::player::PlayerFsmList& seats,
                        pkt::core::player::PlayerFsmList& runningPlayers);

    int getCallsNumber();
    int getRaisesNumber();
    int getBetsOrRaisesNumber();

    std::vector<PlayerPosition> getRaisersPositions();
    std::vector<PlayerPosition> getCallersPositions();

    int getLastRaiserId();
    void setLastRaiserId(int id);

  protected:
    GameState myGameState;
    int myLastRaiserId = -1;

    const pkt::core::player::PlayerFsmList& mySeatsList;
    const pkt::core::player::PlayerFsmList& myRunningPlayersList;
};

} // namespace pkt::core
