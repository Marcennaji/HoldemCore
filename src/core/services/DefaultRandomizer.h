// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/Randomizer.h"

namespace pkt::core
{

/**
 * @brief Default implementation of the randomizer interface using standard library.
 * 
 * This class provides cryptographically secure random number generation
 * for card shuffling, dealing, and other game mechanics that require
 * unpredictable random values.
 */
class DefaultRandomizer : public pkt::core::Randomizer
{
  public:
    void getRand(int minValue, int maxValue, unsigned count, int* out) override;
};
} // namespace pkt::core
