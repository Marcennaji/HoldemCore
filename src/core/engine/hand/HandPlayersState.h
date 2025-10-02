#pragma once

#include "BettingActions.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/Player.h"
#include "typedefs.h"

namespace pkt::core
{

class HandPlayersState
{
  public:
    HandPlayersState(pkt::core::player::PlayerList seats, pkt::core::player::PlayerList actingPlayers);
    ~HandPlayersState() = default;

    const pkt::core::player::PlayerList getSeatsList() const { return m_seatsList; }
    const pkt::core::player::PlayerList getActingPlayersList() const { return m_actingPlayersList; }
    const pkt::core::player::PlayerList getPlayersInHandList() const
    {
        // Create a new list with players in hand (not folded, but might include allin players)
        auto playersInHand = std::make_shared<std::list<std::shared_ptr<pkt::core::player::Player>>>();

        for (const auto& player : *m_seatsList)
        {
            if (player->getLastAction().type != ActionType::Fold)
            {
                playersInHand->push_back(player);
            }
        }

        return playersInHand;
    }
    std::shared_ptr<BettingActions> getBettingActions() const { return m_bettingActions; }
    int getDealerPlayerId() const { return m_dealerPlayerId; }
    int getSmallBlindPlayerId() const { return m_smallBlindPlayerId; }
    int getBigBlindPlayerId() const { return m_bigBlindPlayerId; }

  protected:
    pkt::core::player::PlayerList m_seatsList;         // all players
    pkt::core::player::PlayerList m_actingPlayersList; // all players who have not folded and are not all in
    int m_dealerPlayerId{-1};
    int m_smallBlindPlayerId{-1};
    int m_bigBlindPlayerId{-1};
    std::shared_ptr<BettingActions> m_bettingActions;
};

} // namespace pkt::core