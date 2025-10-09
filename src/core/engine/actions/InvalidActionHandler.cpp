#include "InvalidActionHandler.h"
#include "GameEvents.h"

namespace pkt::core
{

InvalidActionHandler::InvalidActionHandler(const GameEvents& events, ErrorMessageProvider errorProvider,
                                           AutoFoldCallback autoFoldCallback,
                                           Logger& logger)
    : m_events(events), m_errorMessageProvider(std::move(errorProvider)),
      m_autoFoldCallback(std::move(autoFoldCallback)), m_logger(&logger)
{
}

void InvalidActionHandler::handleInvalidAction(const PlayerAction& action)
{
    // Track invalid action attempts
    m_invalidActionCounts[action.playerId]++;

    // Fire event to notify UI of invalid action
    if (m_events.onInvalidPlayerAction)
    {
        std::string reason = m_errorMessageProvider(action);
        m_events.onInvalidPlayerAction(action.playerId, action, reason);
    }

    m_logger->error("Invalid action from player " + std::to_string(action.playerId) + " (attempt " +
                    std::to_string(m_invalidActionCounts[action.playerId]) +
                    "): " + m_errorMessageProvider(action));

    // Check if player should be auto-folded due to repeated invalid actions
    if (shouldAutoFoldPlayer(action.playerId))
    {
        executeAutoFold(action.playerId);
    }
}

void InvalidActionHandler::resetInvalidActionCount(unsigned playerId)
{
    m_invalidActionCounts[playerId] = 0;
}

bool InvalidActionHandler::shouldAutoFoldPlayer(unsigned playerId) const
{
    auto it = m_invalidActionCounts.find(playerId);
    return (it != m_invalidActionCounts.end() && it->second >= MAX_INVALID_ACTIONS);
}

int InvalidActionHandler::getInvalidActionCount(unsigned playerId) const
{
    auto it = m_invalidActionCounts.find(playerId);
    return (it != m_invalidActionCounts.end()) ? it->second : 0;
}

void InvalidActionHandler::executeAutoFold(unsigned playerId)
{
    m_logger->error("Player " + std::to_string(playerId) +
                    " exceeded maximum invalid actions, auto-folding");

    if (m_events.onEngineError)
    {
        m_events.onEngineError("Player " + std::to_string(playerId) + " auto-folded due to repeated invalid actions");
    }

    // Reset counter and trigger auto-fold callback
    resetInvalidActionCount(playerId);
    m_autoFoldCallback(playerId);
}

} // namespace pkt::core