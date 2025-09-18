#pragma once

#include "core/engine/BettingRoundActions.h"
#include "core/engine/model/PlayerAction.h"
#include "core/engine/model/PlayerPosition.h"
#include "core/interfaces/hand/IHandState.h"
#include "core/player/typedefs.h"

#include <optional>
#include <vector>

namespace pkt::core
{

struct BettingRoundHistory
{
    GameState round;
    std::vector<std::pair<unsigned, pkt::core::ActionType>> actions; // playerId, action
};

class BettingActions
{
  public:
    BettingActions(pkt::core::player::PlayerList& seats, pkt::core::player::PlayerList& actingPlayers);

    int getMinRaise(int smallBlind) const;
    int getRoundHighestSet() const;
    void updateRoundHighestSet(int amount);
    void resetRoundHighestSet() { myRoundHighestSet = 0; }
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

    // Hand action history methods
    void recordPlayerAction(GameState round, const pkt::core::PlayerAction& action);
    const std::vector<pkt::core::BettingRoundHistory>& getHandActionHistory() const { return myHandActionHistory; }

  private:
    BettingRoundActions myPreflop;
    BettingRoundActions myFlop;
    BettingRoundActions myTurn;
    BettingRoundActions myRiver;

    int myRoundHighestSet = 0;
    std::optional<unsigned int> myLastRaiserId = std::nullopt;
    int myLastActionPlayerId{-1};
    int myPreviousPlayerId{-1};

    const pkt::core::player::PlayerList& mySeatsList;
    const pkt::core::player::PlayerList& myActingPlayersList;

    // Hand-level action history for chronological tracking
    std::vector<pkt::core::BettingRoundHistory> myHandActionHistory;
};

} // namespace pkt::core
