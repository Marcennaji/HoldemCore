#include "HandPlayersState.h"
#include "core/player/PlayerFsm.h"
#include "core/services/GlobalServices.h"

namespace pkt::core
{
using namespace std;
using namespace pkt::core::player;

HandPlayersState::HandPlayersState(PlayerFsmList seats, PlayerFsmList actingPlayers)
    : mySeatsList(seats), myActingPlayersList(actingPlayers)

{
    myBettingActions = make_shared<BettingActions>(mySeatsList, myActingPlayersList);
}

} // namespace pkt::core
