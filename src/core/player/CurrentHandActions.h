// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.



#pragma once

#include "core/engine/model/GameState.h"
#include "core/engine/model/PlayerAction.h"
#include "core/ports/Logger.h"

#include <map>
#include <memory>
#include <vector>

namespace pkt::core::player
{

/**
 * @brief Tracks and manages player actions during the current poker hand.
 * 
 * Maintains a record of all actions taken by a player during different
 * game states (preflop, flop, turn, river) within the current hand,
 * supporting action history analysis and strategy evaluation.
 */
class CurrentHandActions
{
  public:
    CurrentHandActions();
    explicit CurrentHandActions(std::shared_ptr<Logger> logger);
    ~CurrentHandActions() = default;

    void reset();

    void addAction(const GameState& state, const PlayerAction& action);
    const std::vector<PlayerAction>& getActions(const GameState& state) const { return m_actionsByState.at(state); }

    int getActionsNumber(const GameState& state, const ActionType& actionType) const;

    int getLastBetAmount(const GameState& state) const;
    int getRoundTotalBetAmount(const GameState& state) const;
    int getHandTotalBetAmount() const;

    PlayerAction getLastAction() const { return m_lastAction; }

    void writeActionsToLog() const;
    void writeActionsToLog(Logger& logger) const;

  private:
    std::shared_ptr<Logger> m_logger;

    std::map<GameState, std::vector<PlayerAction>> m_actionsByState;
    PlayerAction m_lastAction{-1, ActionType::None, 0};
};

} // namespace pkt::core::player
