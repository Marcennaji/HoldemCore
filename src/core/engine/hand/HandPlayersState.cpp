#include "HandPlayersState.h"
#include "core/player/Player.h"

namespace pkt::core
{
using namespace std;
using namespace pkt::core::player;

HandPlayersState::HandPlayersState(PlayerList seats, PlayerList actingPlayers)
    : m_seatsList(seats), m_actingPlayersList(actingPlayers)

{
    m_bettingActions = make_shared<BettingActions>(m_seatsList, m_actingPlayersList);
}

} // namespace pkt::core
