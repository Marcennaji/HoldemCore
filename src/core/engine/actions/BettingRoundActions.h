// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once

#include <optional>
#include "core/engine/model/GameState.h"
#include "core/engine/model/PlayerPosition.h"
#include "core/engine/hand/HandState.h"
#include "core/player/typedefs.h"

#include <vector>
#include <memory>

namespace pkt::core
{
/**
 * @brief Tracks and analyzes betting actions within a single betting round.
 * 
 * This class provides analysis of betting patterns during a specific round
 * (preflop, flop, turn, or river), tracking raises, calls, and player positions
 * to support game logic and statistical analysis.
 */
class BettingRoundActions
{
  public:
    BettingRoundActions(GameState gameState, pkt::core::player::PlayerList& seats,
                        pkt::core::player::PlayerList& actingPlayers);

    int getCallsNumber();
    int getRaisesNumber();
    int getBetsOrRaisesNumber();

    std::vector<PlayerPosition> getRaisersPositions();
    std::vector<PlayerPosition> getCallersPositions();

    std::shared_ptr<pkt::core::player::Player> getLastRaiser();
    void setLastRaiser(std::shared_ptr<pkt::core::player::Player> player);

  protected:
    GameState m_gameState;
    std::shared_ptr<pkt::core::player::Player> m_lastRaiser = nullptr;

    const pkt::core::player::PlayerList& m_seatsList;
    const pkt::core::player::PlayerList& m_actingPlayersList;
};

} // namespace pkt::core
