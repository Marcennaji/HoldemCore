// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#define NOMINMAX // for Windows

#include "Randomizer.h"
#include <core/interfaces/ILogger.h>

#include <algorithm>
#include <memory>
#include <random>

using namespace std;

std::random_device g_rand_device;
std::mt19937 g_rand_engine(g_rand_device());

static inline void InitRandState()
{
    // No need for shared_ptr in this case
}
namespace pkt::core
{

void Randomizer::ShuffleArrayNonDeterministic(vector<int>& inout)
{
    InitRandState();
    shuffle(inout.begin(), inout.end(), g_rand_engine);
}

void Randomizer::GetRand(int minValue, int maxValue, unsigned count, int* out)
{
    InitRandState();
    uniform_int_distribution<int> dist(minValue, maxValue);

    int* startPtr = out;
    for (unsigned i = 0; i < count; i++)
    {
        *startPtr++ = dist(g_rand_engine);
    }
}
} // namespace pkt::core
