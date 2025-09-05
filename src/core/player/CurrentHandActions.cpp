

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
}

void CurrentHandActions::reset()
{
    for (auto& [state, actions] : myActionsByState)
        actions.clear();
}

int CurrentHandActions::getActionsNumber(const GameState& state, const ActionType& actionType) const
{
    int count = 0;
    for (const auto& action : myActionsByState.at(state))
    {
        if (action.type == actionType)
            count++;
    }
    return count;
}

int CurrentHandActions::getLastBetAmount(const GameState& state) const
{
    const auto& actions = myActionsByState.at(state);
    for (auto it = actions.rbegin(); it != actions.rend(); ++it)
    {
        if (it->type == ActionType::Bet || it->type == ActionType::Raise || it->type == ActionType::Allin)
            return it->amount;
    }
    return 0;
}

int CurrentHandActions::getRoundTotalBetAmount(const GameState& state) const
{
    int total = 0;
    for (const auto& action : myActionsByState.at(state))
    {
        if (action.type == ActionType::Bet || action.type == ActionType::Raise || action.type == ActionType::Call ||
            action.type == ActionType::Allin)
        {
            total += action.amount;
        }
    }
    return total;
}

int CurrentHandActions::getHandTotalBetAmount() const
{
    int total = 0;
    for (const auto& [state, actions] : myActionsByState)
    {
        for (const auto& action : actions)
        {
            if (action.type == ActionType::Bet || action.type == ActionType::Raise || action.type == ActionType::Call ||
                action.type == ActionType::Allin)
            {
                total += action.amount;
            }
        }
    }
    return total;
}
} // namespace pkt::core::player
