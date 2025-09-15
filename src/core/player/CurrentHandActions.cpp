

#include "CurrentHandActions.h"
#include <core/engine/model/GameState.h>
#include "core/services/GlobalServices.h"

#include <cassert>

namespace pkt::core::player
{
using namespace pkt::core;

void checkActionAmount(const PlayerAction& action)
{
    if (action.type == ActionType::Bet || action.type == ActionType::Raise)
    {
        // NB. an amount is only required for a bet or a raise
        if (action.amount == 0)
        {
            GlobalServices::instance().logger().error("Action amount missing for action type " +
                                                      std::string(actionTypeToString(action.type)));
            assert(false);
        }
    }
}

CurrentHandActions::CurrentHandActions()
{
    myActionsByState[GameState::Preflop] = {};
    myActionsByState[GameState::Flop] = {};
    myActionsByState[GameState::Turn] = {};
    myActionsByState[GameState::River] = {};
    myActionsByState[GameState::PostRiver] = {};
    myActionsByState[GameState::None] = {};
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
    for (auto action = actions.rbegin(); action != actions.rend(); ++action)
    {
        if (action->type == ActionType::Bet || action->type == ActionType::Raise || action->type == ActionType::Allin)
        {
            if (action->type == ActionType::Bet || action->type == ActionType::Raise)
            {
                checkActionAmount(*action);
            }
        }
        return action->amount;
    }
    return 0;
}

int CurrentHandActions::getRoundTotalBetAmount(const GameState& state) const
{
    int total = 0;
    for (const auto& action : myActionsByState.at(state))
    {
        if (action.type == ActionType::Bet || action.type == ActionType::Raise)
        {
            checkActionAmount(action);
        }
        total += action.amount;
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
            if (action.type == ActionType::Bet || action.type == ActionType::Raise)
            {
                checkActionAmount(action);
            }
            total += action.amount;
        }
    }
    return total;
}
void CurrentHandActions::writeActionsToLog() const
{
    for (const auto& [state, actions] : myActionsByState)
    {
        GlobalServices::instance().logger().error("List of player's actions for state: " + gameStateToString(state));
        for (const auto& action : actions)
        {
            GlobalServices::instance().logger().error("Player " + std::to_string(action.playerId) + " performed " +
                                                      actionTypeToString(action.type) +
                                                      " with amount = " + std::to_string(action.amount));
        }
    }
}

void CurrentHandActions::addAction(const GameState& state, const PlayerAction& action)
{
    myActionsByState[state].push_back(action);
    myLastAction = action;
}

} // namespace pkt::core::player
