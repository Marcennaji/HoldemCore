#pragma once

#include "core/engine/BettingRoundActions.h"
#include "core/engine/model/PlayerPosition.h"
#include "core/interfaces/hand/IHandState.h"
#include "core/player/typedefs.h"

#include <optional>
#include <vector>

namespace pkt::core
{
class BettingActions
{
  public:
    BettingActions(pkt::core::player::PlayerFsmList& seats, pkt::core::player::PlayerFsmList& runningPlayers);
    bool isRoundComplete(const HandFsm& hand) const;
    int getHighestSet() const;
    int getMinRaise(int smallBlind) const;
    void updateHighestSet(int amount);
    void resetHighestSet() { myHighestSet = 0; }
    void recordRaise(unsigned int id) { myLastRaiserId = id; }

    std::optional<unsigned int> getLastRaiserId() const { return myLastRaiserId; }

    void resetRaiser() { myLastRaiserId = std::nullopt; }

    BettingRoundActions& getPreflop() { return myPreflop; }
    BettingRoundActions& getFlop() { return myFlop; }
    BettingRoundActions& getTurn() { return myTurn; }
    BettingRoundActions& getRiver() { return myRiver; }

    std::vector<PlayerPosition> getRaisersPositions();
    std::vector<PlayerPosition> getCallersPositions();
    int getLastRaiserId();
    void setLastActionPlayerId(int theValue);
    int getLastActionPlayerId() const { return myLastActionPlayerId; }

  private:
    BettingRoundActions myPreflop;
    BettingRoundActions myFlop;
    BettingRoundActions myTurn;
    BettingRoundActions myRiver;

    bool haveAllPlayersCalledOrFolded(const HandFsm& hand) const;
    bool isOnlyOnePlayerRemaining(const HandFsm& hand) const;
    int myHighestSet = 0;
    std::optional<unsigned int> myLastRaiserId = std::nullopt;
    int myLastActionPlayerId;
    int myPreviousPlayerId{-1};

    const pkt::core::player::PlayerFsmList& mySeatsList;
    const pkt::core::player::PlayerFsmList& myRunningPlayersList;
};

} // namespace pkt::core
