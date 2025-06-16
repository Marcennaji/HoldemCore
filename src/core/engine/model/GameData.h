// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
/* Game data. */

#pragma once

#include "TableProfile.h"

namespace pkt::core
{
struct GameData
{
    GameData() : maxNumberOfPlayers(0), startMoney(0), firstSmallBlind(0), guiSpeed(0) {}
    TableProfile tableProfile;
    int maxNumberOfPlayers;
    int startMoney;
    int firstSmallBlind;
    int guiSpeed;
};
} // namespace pkt::core
