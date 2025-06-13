/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the MIT License                                     *


 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * MIT License for more details.                                             *
 *                                                                           *
 * You should have received a copy of the MIT License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/
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
