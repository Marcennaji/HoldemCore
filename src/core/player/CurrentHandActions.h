
#pragma once

#include "core/engine/model/GameState.h"
#include "core/engine/model/PlayerAction.h"

#include <map>
#include <vector>

namespace pkt::core::player
{

class CurrentHandActions
{
  public:
    CurrentHandActions();
    ~CurrentHandActions() = default;

    void reset();

    std::vector<PlayerAction>& getActions(const GameState& state) { return myActionsByState[state]; }
    const std::vector<PlayerAction>& getActions(const GameState& state) const { return myActionsByState.at(state); }

    int getActionsNumber(const GameState& state, const ActionType& actionType) const;

    int getLastBetAmount(const GameState& state) const;
    int getRoundTotalBetAmount(const GameState& state) const;
    int getHandTotalBetAmount() const;

  private:
    std::map<GameState, std::vector<PlayerAction>> myActionsByState;
};

} // namespace pkt::core::player
