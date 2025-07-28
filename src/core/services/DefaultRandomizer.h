// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/IRandomizer.h"

namespace pkt::core
{

class DefaultRandomizer : public pkt::core::IRandomizer
{
  public:
    void getRand(int minValue, int maxValue, unsigned count, int* out) override;
};
} // namespace pkt::core
