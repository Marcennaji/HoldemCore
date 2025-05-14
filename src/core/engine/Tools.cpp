/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU Affero General Public License as            *
 * published by the Free Software Foundation, either version 3 of the        *
 * License, or (at your option) any later version.                           *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU Affero General Public License for more details.                       *
 *                                                                           *
 * You should have received a copy of the GNU Affero General Public License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#define NOMINMAX // for Windows

#include "tools.h"
#include <core/engine/ILogger.h>
#pragma message("Included LogHelper.h successfully")

#include <memory>
#include <random>
#include <algorithm>

using namespace std;

std::random_device g_rand_device;
std::mt19937 g_rand_engine(g_rand_device());

static inline void InitRandState() {
    // No need for shared_ptr in this case
}

void Tools::ShuffleArrayNonDeterministic(vector<int>& inout) {
    InitRandState();
    shuffle(inout.begin(), inout.end(), g_rand_engine);
}

void Tools::GetRand(int minValue, int maxValue, unsigned count, int* out) {
    InitRandState();
    uniform_int_distribution<int> dist(minValue, maxValue);

    int* startPtr = out;
    for (unsigned i = 0; i < count; i++) {
        *startPtr++ = dist(g_rand_engine);
    }
}
