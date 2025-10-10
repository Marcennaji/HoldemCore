// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/engine/model/PlayerAction.h"
#include "core/interfaces/Logger.h"
#include "core/engine/GameEvents.h"

namespace pkt::core
{

// Forward declarations
class Hand;
class ActionValidator;
class InvalidActionHandler;
class HandPlayersManager;

/**
 * @brief Handles player action processing and validation for poker hands.
 * 
 * This class extracts action handling responsibilities from the Hand class,
 * providing focused action processing without coupling to game state management.
 * 
 * Responsibilities:
 * - Player action processing and validation
 * - Invalid action handling and auto-fold logic
 * - Action validation error generation
 * - Coordination with ActionValidator and InvalidActionHandler
 * - Action application and state transition triggering
 */
class HandActionHandler
{
  public:
    HandActionHandler(const GameEvents& events,
                      Logger& logger,
                      std::unique_ptr<ActionValidator> actionValidator,
                      std::unique_ptr<InvalidActionHandler> invalidActionHandler);
    ~HandActionHandler() = default;

    // Main action processing interface
    void handlePlayerAction(PlayerAction action, Hand& hand);

    // Action validation and error handling
    std::string getActionValidationError(const PlayerAction& action, const Hand& hand, const HandPlayersManager& playersManager) const;
    PlayerAction getDefaultActionForPlayer(unsigned playerId) const;
    
    // Auto-fold handling
    void handleAutoFold(unsigned playerId, Hand& hand);
    
  private:
    // Internal action processing methods
    void processValidAction(const PlayerAction& action, Hand& hand);
    
    const GameEvents& m_events;
    Logger* m_logger;
    std::unique_ptr<ActionValidator> m_actionValidator;
    std::unique_ptr<InvalidActionHandler> m_invalidActionHandler;
};

} // namespace pkt::core