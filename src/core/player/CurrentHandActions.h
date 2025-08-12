
#pragma once

#include <core/engine/model/PlayerAction.h>
#include "core/engine/model/GameState.h"

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

    std::vector<ActionType>& getActions(const GameState& state) { return myActionsByState[state]; };
    const std::vector<ActionType>& getActions(const GameState& state) const { return myActionsByState.at(state); };
    int getActionsNumber(const GameState& state, const ActionType& actionType) const;

  protected:
    std::map<GameState, std::vector<pkt::core::ActionType>> myActionsByState;
};
} // namespace pkt::core::player