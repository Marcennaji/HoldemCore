// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once
#include <vector>

namespace pkt::core
{

class Randomizer
{
  public:
    static void GetRand(int minValue, int maxValue, unsigned count, int* out);
};
} // namespace pkt::core
