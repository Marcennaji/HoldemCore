#include "HandPlayersState.h"
#include "core/player/Player.h"

namespace pkt::core
{
using namespace std;
using namespace pkt::core::player;

HandPlayersState::HandPlayersState(PlayerList seats, PlayerList actingPlayers)
    : mySeatsList(seats), myActingPlayersList(actingPlayers)

{
    myBettingActions = make_shared<BettingActions>(mySeatsList, myActingPlayersList);
}

} // namespace pkt::core
