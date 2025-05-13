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

#pragma once

enum class EngineError {
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
