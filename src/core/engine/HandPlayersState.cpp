#include "HandPlayersState.h"
#include "core/player/PlayerFsm.h"
#include "core/player/deprecated/Player.h"
#include "core/services/GlobalServices.h"

namespace pkt::core
{
using namespace std;
using namespace pkt::core::player;

HandPlayersState::HandPlayersState(PlayerFsmList seats, PlayerFsmList runningPlayers)
    : mySeatsList(seats), myRunningPlayersList(runningPlayers)

{
    myBettingState = make_shared<BettingState>(mySeatsList, myRunningPlayersList);
}

} // namespace pkt::core
