// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#define NOMINMAX // for Windows

#include "DefaultRandomizer.h"

#include <algorithm>
#include <memory>
#include <random>

using namespace std;

std::random_device g_rand_device;
std::mt19937 g_rand_engine(g_rand_device());

namespace pkt::core
{

void DefaultRandomizer::getRand(int minValue, int maxValue, unsigned count, int* out)
{
    uniform_int_distribution<int> dist(minValue, maxValue);

    int* startPtr = out;
    for (unsigned i = 0; i < count; i++)
    {
        *startPtr++ = dist(g_rand_engine);
    }
}
} // namespace pkt::core
