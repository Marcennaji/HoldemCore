// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once

#include "core/engine/model/PlayerAction.h"

namespace pkt::core
{
/**
 * @brief Abstract interface for handling player actions during a poker hand.
 * 
 * Defines the contract for processing and responding to player actions
 * (fold, call, raise, etc.) within the context of an active poker hand.
 */
class HandPlayerAction
{
  public:
    virtual ~HandPlayerAction() = default;
    virtual void handlePlayerAction(PlayerAction action) = 0;
};
} // namespace pkt::core
