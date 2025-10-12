// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once

namespace pkt::core
{
/**
 * @brief Abstract interface for random number generation functionality.
 * 
 * Provides a contract for random number generators used throughout the
 * poker engine, enabling deterministic testing with fake randomizers
 * and pluggable random number generation strategies.
 */
class Randomizer
{
  public:
    virtual void getRand(int minValue, int maxValue, unsigned count, int* out) = 0;
    virtual ~Randomizer() = default;
};
} // namespace pkt::core
