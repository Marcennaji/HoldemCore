// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once

#include "core/engine/model/GameState.h"

#include <string>

namespace pkt::core
{

class Hand;

/**
 * @brief Abstract base class for representing different states of a poker hand.
 * 
 * Implements the State pattern for managing poker hand phases (preflop, flop,
 * turn, river, showdown), defining the interface for state transitions and
 * behavior within each phase of the game.
 */
class HandState
{
  public:
    virtual ~HandState() = default;

    virtual void enter(Hand& hand) = 0;
    virtual void exit(Hand& hand) = 0;
    virtual bool isTerminal() const { return false; }

    virtual const GameState getGameState() const = 0;
};
} // namespace pkt::core
