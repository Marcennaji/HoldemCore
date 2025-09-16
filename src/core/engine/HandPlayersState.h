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

    const pkt::core::player::PlayerList getSeatsList() const { return mySeatsList; }
    const pkt::core::player::PlayerList getActingPlayersList() const { return myActingPlayersList; }
    const pkt::core::player::PlayerList getPlayersInHandList() const
    {
        // Create a new list with players in hand (not folded, but might include allin players)
        auto playersInHand = std::make_shared<std::list<std::shared_ptr<pkt::core::player::Player>>>();

        for (const auto& player : *mySeatsList)
        {
            if (player->getLastAction().type != ActionType::Fold)
            {
                playersInHand->push_back(player);
            }
        }

        return playersInHand;
    }
    std::shared_ptr<BettingActions> getBettingActions() const { return myBettingActions; }
    int getDealerPlayerId() const { return myDealerPlayerId; }
    int getSmallBlindPlayerId() const { return mySmallBlindPlayerId; }
    int getBigBlindPlayerId() const { return myBigBlindPlayerId; }

  protected:
    pkt::core::player::PlayerList mySeatsList;         // all players
    pkt::core::player::PlayerList myActingPlayersList; // all players who have not folded and are not all in
    int myDealerPlayerId{-1};
    int mySmallBlindPlayerId{-1};
    int myBigBlindPlayerId{-1};
    std::shared_ptr<BettingActions> myBettingActions;
};

} // namespace pkt::core