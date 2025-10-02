

#include "CurrentHandActions.h"
#include <core/engine/model/GameState.h>
#include <core/services/ServiceContainer.h>
#include "core/player/Helpers.h"

#include <cassert>

namespace pkt::core::player
{
using namespace pkt::core;

void checkActionAmount(const PlayerAction& action, pkt::core::ServiceContainer& services)
{
    if (action.type == ActionType::Bet || action.type == ActionType::Raise)
    {
        // NB. an amount is only required for a bet or a raise
        if (action.amount == 0)
        {
            std::string errorMsg =
                "Action amount missing for action type " + std::string(actionTypeToString(action.type));

            services.logger().error(errorMsg);
            assert(false);
        }
    }
}

CurrentHandActions::CurrentHandActions()
{
    m_actionsByState[GameState::Preflop] = {};
    m_actionsByState[GameState::Flop] = {};
    m_actionsByState[GameState::Turn] = {};
    m_actionsByState[GameState::River] = {};
    m_actionsByState[GameState::PostRiver] = {};
    m_actionsByState[GameState::None] = {};
}

CurrentHandActions::CurrentHandActions(std::shared_ptr<pkt::core::ServiceContainer> serviceContainer)
    : m_services(serviceContainer)
{
    m_actionsByState[GameState::Preflop] = {};
    m_actionsByState[GameState::Flop] = {};
    m_actionsByState[GameState::Turn] = {};
    m_actionsByState[GameState::River] = {};
    m_actionsByState[GameState::PostRiver] = {};
    m_actionsByState[GameState::None] = {};
}

void CurrentHandActions::ensureServicesInitialized() const
{
    if (!m_services)
    {
        m_services = std::make_shared<pkt::core::AppServiceContainer>();
    }
}

void CurrentHandActions::reset()
{
    for (auto& [state, actions] : m_actionsByState)
        actions.clear();
}

int CurrentHandActions::getActionsNumber(const GameState& state, const ActionType& actionType) const
{
    int count = 0;
    for (const auto& action : m_actionsByState.at(state))
    {
        if (action.type == actionType)
            count++;
    }
    return count;
}

int CurrentHandActions::getLastBetAmount(const GameState& state) const
{
    ensureServicesInitialized();

    const auto& actions = m_actionsByState.at(state);
    for (auto action = actions.rbegin(); action != actions.rend(); ++action)
    {
        if (action->type == ActionType::Bet || action->type == ActionType::Raise || action->type == ActionType::Allin)
        {
            if (action->type == ActionType::Bet || action->type == ActionType::Raise)
            {
                checkActionAmount(*action, *m_services);
            }
        }
        return action->amount;
    }
    return 0;
}

int CurrentHandActions::getRoundTotalBetAmount(const GameState& state) const
{
    ensureServicesInitialized();

    int total = 0;
    for (const auto& action : m_actionsByState.at(state))
    {
        if (action.type == ActionType::Bet || action.type == ActionType::Raise)
        {
            checkActionAmount(action, *m_services);
        }
        total += action.amount;
    }

    return total;
}

int CurrentHandActions::getHandTotalBetAmount() const
{
    ensureServicesInitialized();

    int total = 0;
    for (const auto& [state, actions] : m_actionsByState)
    {
        for (const auto& action : actions)
        {
            if (action.type == ActionType::Bet || action.type == ActionType::Raise)
            {
                checkActionAmount(action, *m_services);
            }
            total += action.amount;
        }
    }
    return total;
}
void CurrentHandActions::writeActionsToLog() const
{
    ensureServicesInitialized();
    writeActionsToLog(m_services->logger());
}

void CurrentHandActions::writeActionsToLog(Logger& logger) const
{
    for (const auto& [state, actions] : m_actionsByState)
    {
        logger.error("List of player's actions for state: " + gameStateToString(state));
        for (const auto& action : actions)
        {
            logger.error("Player " + std::to_string(action.playerId) + " performed " + actionTypeToString(action.type) +
                         " with amount = " + std::to_string(action.amount));
        }
    }
}

void CurrentHandActions::addAction(const GameState& state, const PlayerAction& action)
{
    m_actionsByState[state].push_back(action);
    m_lastAction = action;
}

} // namespace pkt::core::player
