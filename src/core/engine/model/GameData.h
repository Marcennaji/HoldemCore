// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
/* Game data. */

#pragma once

#include "TableProfile.h"

namespace pkt::core
{
struct GameData
{
    GameData() = default;
    TableProfile tableProfile;
    int maxNumberOfPlayers{0};
    int startMoney{0};
    int firstSmallBlind{0};
    int guiSpeed{0};
};
} // namespace pkt::core
