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

#pragma once

namespace pkt::core
{

enum class EngineError
{
    SEAT_NOT_FOUND = 10001,
    ACTIVE_PLAYER_NOT_FOUND = 10002,
    RUNNING_PLAYER_NOT_FOUND = 10003,
    DEALER_NOT_FOUND = 10004,
    CURRENT_PLAYER_NOT_FOUND = 10005,
    NEXT_DEALER_NOT_FOUND = 10010,
    NEXT_ACTIVE_PLAYER_NOT_FOUND = 10011,
    FORMER_RUNNING_PLAYER_NOT_FOUND = 10012,
    PLAYER_ACTION_ERROR = 10030
};
} // namespace pkt::core
