// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "HandActionHandler.h"
#include "Hand.h"
#include "core/engine/actions/ActionValidator.h"
#include "core/engine/actions/InvalidActionHandler.h"
#include "core/engine/actions/ActionApplier.h"

namespace pkt::core
{

HandActionHandler::HandActionHandler(const GameEvents& events,
                                     Logger& logger,
                                     std::unique_ptr<ActionValidator> actionValidator,
                                     std::unique_ptr<InvalidActionHandler> invalidActionHandler)
    : m_events(events), m_logger(&logger),
      m_actionValidator(std::move(actionValidator)),
      m_invalidActionHandler(std::move(invalidActionHandler))
{
}

void HandActionHandler::handlePlayerAction(PlayerAction action, Hand& hand)
{
    auto* processor = hand.getActionProcessor();

    if (!processor || !processor->isActionAllowed(hand, action))
    {
        m_invalidActionHandler->handleInvalidAction(action);
        return;
    }

    processValidAction(action, hand);
}

std::string HandActionHandler::getActionValidationError(const PlayerAction& action, const Hand& hand, const HandPlayersManager& playersManager) const
{
    if (hand.getStateManager()->isTerminal())
    {
        return "Game state is terminal";
    }

    auto* processor = hand.getStateManager()->getActionProcessor();
    if (!processor)
    {
        return "Current game state does not accept player actions";
    }

    auto player = playersManager.validatePlayer(action.playerId);
    if (!player)
    {
        return "Player not found in active players list";
    }

    // Check if it's the player's turn
    auto currentPlayer = processor->getNextPlayerToAct(hand);
    if (!currentPlayer || currentPlayer->getId() != action.playerId)
    {
        return "It's not this player's turn to act";
    }

    // Use the comprehensive ActionValidator with a detailed reason
    {
        std::string reason;
        if (!m_actionValidator->validatePlayerActionWithReason(hand.getActingPlayersList(), action, *hand.getBettingActions(),
                                                               hand.getSmallBlind(), hand.getStateManager()->getGameState(), reason))
        {
            return reason.empty() ? std::string("Action validation failed.") : reason;
        }
    }

    return ""; // Empty string means action is valid
}

PlayerAction HandActionHandler::getDefaultActionForPlayer(unsigned playerId) const
{
    PlayerAction defaultAction;
    defaultAction.playerId = playerId;
    defaultAction.type = ActionType::Fold; // Default to fold for safety
    defaultAction.amount = 0;
    return defaultAction;
}

void HandActionHandler::handleAutoFold(unsigned playerId, Hand& hand)
{
    m_logger->error("Player " + std::to_string(playerId) +
                   " exceeded maximum invalid actions, auto-folding");

    // If the game state is terminal, don't try to process any actions
    if (hand.getStateManager()->isTerminal())
    {
        m_logger->error("Cannot auto-fold player " + std::to_string(playerId) + " - game state is terminal");

        if (m_events.onEngineError)
        {
            m_events.onEngineError("Player " + std::to_string(playerId) +
                                   " attempted action in terminal state - no auto-fold processed");
        }
        return;
    }

    // Create a fold action as default
    PlayerAction autoFoldAction = getDefaultActionForPlayer(playerId);

    if (m_events.onEngineError)
    {
        m_events.onEngineError("Player " + std::to_string(playerId) + " auto-folded due to repeated invalid actions");
    }

    // Recursively call with the auto-fold action
    handlePlayerAction(autoFoldAction, hand);
}

void HandActionHandler::processValidAction(const PlayerAction& action, Hand& hand)
{
    try
    {
        // Reset invalid action count on successful action
        m_invalidActionHandler->resetInvalidActionCount(action.playerId);

        ActionApplier::apply(hand, action, *m_logger);

        // Delegate state transition to HandStateManager
        hand.getStateManager()->transitionToNextState(hand);
    }
    catch (const std::exception& e)
    {
        if (m_events.onEngineError)
        {
            m_events.onEngineError("Error processing player action: " + std::string(e.what()));
        }

        m_logger->error("Error in handlePlayerAction: " + std::string(e.what()));

        // Re-throw critical errors
        throw;
    }
}

} // namespace pkt::core