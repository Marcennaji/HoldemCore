#pragma once

#include "BettingState.h"
#include "typedefs.h"

namespace pkt::core
{

class HandPlayersState
{
  public:
    HandPlayersState(pkt::core::player::PlayerFsmList seats, pkt::core::player::PlayerFsmList runningPlayers);
    ~HandPlayersState() = default;

    const pkt::core::player::PlayerFsmList getSeatsList() const { return mySeatsList; }
    const pkt::core::player::PlayerFsmList getRunningPlayersList() const { return myRunningPlayersList; }
    std::shared_ptr<BettingState> getBettingState() const { return myBettingState; }

  protected:
    pkt::core::player::PlayerFsmList mySeatsList;          // all players
    pkt::core::player::PlayerFsmList myRunningPlayersList; // all players who have not folded and are not all in
    int myDealerPlayerId{-1};
    int mySmallBlindPlayerId{-1};
    int myBigBlindPlayerId{-1};
    std::shared_ptr<BettingState> myBettingState;
};

} // namespace pkt::core