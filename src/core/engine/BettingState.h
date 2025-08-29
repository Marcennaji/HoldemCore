#pragma once

#include <optional>
#include "core/engine/model/PlayerPosition.h"
#include "core/interfaces/hand/IHandState.h"
#include "core/player/typedefs.h"

#include <vector>

namespace pkt::core
{
class BettingState
{
  public:
    BettingState(pkt::core::player::PlayerFsmList& seats, pkt::core::player::PlayerFsmList& runningPlayers);
    bool isRoundComplete(const HandFsm& hand) const;
    int getHighestSet() const;
    void updateHighestSet(int amount);
    void resetHighestSet() { myHighestSet = 0; }
    void recordRaise(unsigned int id) { myLastRaiserId = id; }

    std::optional<unsigned int> getLastRaiserId() const { return myLastRaiserId; }

    void resetRaiser() { myLastRaiserId = std::nullopt; }

    int getPreflopCallsNumber();
    int getPreflopRaisesNumber();
    int getFlopBetsOrRaisesNumber();
    int getTurnBetsOrRaisesNumber();
    int getRiverBetsOrRaisesNumber();

    std::vector<PlayerPosition> getRaisersPositions();
    std::vector<PlayerPosition> getCallersPositions();
    int getLastRaiserId();
    int getPreflopLastRaiserId();
    void setPreflopLastRaiserId(int id);
    int getFlopLastRaiserId();
    void setFlopLastRaiserId(int id);
    int getTurnLastRaiserId();
    void setTurnLastRaiserId(int id);
    void setLastActionPlayerId(int theValue);
    int getLastActionPlayerId() const { return myLastActionPlayerId; }

  protected:
    bool haveAllPlayersCalledOrFolded(const HandFsm& hand) const;
    bool isOnlyOnePlayerRemaining(const HandFsm& hand) const;
    int myHighestSet = 0;
    std::optional<unsigned int> myLastRaiserId = std::nullopt;
    int myLastActionPlayerId;
    int myPreviousPlayerId{-1};
    int myPreflopLastRaiserId;
    int myFlopLastRaiserId;
    int myTurnLastRaiserId;

    const pkt::core::player::PlayerFsmList& mySeatsList;
    const pkt::core::player::PlayerFsmList& myRunningPlayersList;
};

} // namespace pkt::core
