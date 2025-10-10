// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "BettingRoundActions.h"
#include "core/engine/model/PlayerAction.h"
#include "core/engine/model/PlayerPosition.h"
#include "core/engine/hand/HandState.h"
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

/**
 * @brief Manages betting actions and validation during poker betting rounds.
 * 
 * This class handles the logic for determining valid betting actions,
 * calculating minimum raises, tracking betting history, and validating
 * player actions within the context of the current betting round.
 */
class BettingActions
{
  public:
    BettingActions(pkt::core::player::PlayerList& seats, pkt::core::player::PlayerList& actingPlayers);

    int getMinRaise(int smallBlind) const;
    int getRoundHighestSet() const;
    void updateRoundHighestSet(int amount);
    void resetRoundHighestSet() { m_roundHighestSet = 0; }
    void recordRaise(unsigned int id) { m_lastRaiserId = id; }

    std::optional<unsigned int> getLastRaiserId() const { return m_lastRaiserId; }

    void resetRaiser() { m_lastRaiserId = std::nullopt; }

    BettingRoundActions& getPreflop() { return m_preflop; }
    BettingRoundActions& getFlop() { return m_flop; }
    BettingRoundActions& getTurn() { return m_turn; }
    BettingRoundActions& getRiver() { return m_river; }

    std::vector<PlayerPosition> getRaisersPositions();
    std::vector<PlayerPosition> getCallersPositions();
    int getLastRaiserId();
    void setLastActionPlayerId(int theValue);
    int getLastActionPlayerId() const { return m_lastActionPlayerId; }

    // Hand action history methods
    void recordPlayerAction(GameState round, const pkt::core::PlayerAction& action);
    const std::vector<pkt::core::BettingRoundHistory>& getHandActionHistory() const { return m_handActionHistory; }

  private:
    BettingRoundActions m_preflop;
    BettingRoundActions m_flop;
    BettingRoundActions m_turn;
    BettingRoundActions m_river;

    int m_roundHighestSet = 0;
    std::optional<unsigned int> m_lastRaiserId = std::nullopt;
    int m_lastActionPlayerId{-1};
    int m_previousPlayerId{-1};

    const pkt::core::player::PlayerList& m_seatsList;
    const pkt::core::player::PlayerList& m_actingPlayersList;

    // Hand-level action history for chronological tracking
    std::vector<pkt::core::BettingRoundHistory> m_handActionHistory;
};

} // namespace pkt::core
