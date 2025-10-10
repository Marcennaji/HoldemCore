// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once

#include "core/engine/model/PlayerAction.h"

#include <string>

namespace pkt::core::player
{

struct CurrentHandContext;

/**
 * @brief Abstract base class for implementing poker player strategies.
 * 
 * Defines the interface for player decision-making logic, allowing different
 * AI strategies to be implemented and plugged into players for automated
 * poker gameplay and testing scenarios.
 */
class PlayerStrategy
{
  public:
    virtual ~PlayerStrategy() = default;

    virtual pkt::core::PlayerAction decideAction(const CurrentHandContext& ctx) = 0;
};

} // namespace pkt::core::player
