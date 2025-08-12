

#include "CurrentHandActions.h"

#include <core/engine/model/GameState.h>

namespace pkt::core::player
{

CurrentHandActions::CurrentHandActions()
{
    myActionsByState[GameState::Preflop] = {};
    myActionsByState[GameState::Flop] = {};
    myActionsByState[GameState::Turn] = {};
    myActionsByState[GameState::River] = {};
};

void CurrentHandActions::reset()
{
    myActionsByState[GameState::Preflop].clear();
    myActionsByState[GameState::Flop].clear();
    myActionsByState[GameState::Turn].clear();
    myActionsByState[GameState::River].clear();
}

int CurrentHandActions::getActionsNumber(const GameState& state, const ActionType& actionType) const
{
    int count = 0;

    for (const auto& action : myActionsByState.at(state))
    {
        if (action == actionType)
        {
            count++;
        }
    }

    return count;
}
} // namespace pkt::core::player
