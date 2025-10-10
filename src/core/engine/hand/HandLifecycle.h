// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once

namespace pkt::core
{

/**
 * @brief Abstract interface defining the lifecycle of a poker hand.
 * 
 * Defines the contract for managing the complete lifecycle of a poker hand,
 * from initialization through the game loop execution to final cleanup
 * and result processing.
 */
class HandLifecycle
{
  public:
    virtual ~HandLifecycle() = default;
    virtual void initialize() = 0;
    virtual void runGameLoop() = 0;
    virtual void end() = 0;
};
} // namespace pkt::core
