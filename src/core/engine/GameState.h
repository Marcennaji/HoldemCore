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
/* Game data. */

#pragma once


enum GameState {
    GAME_STATE_PREFLOP = 0,
    GAME_STATE_FLOP,
    GAME_STATE_TURN,
    GAME_STATE_RIVER,
    GAME_STATE_POST_RIVER,
    GAME_STATE_PREFLOP_SMALL_BLIND = 0xF0,
    GAME_STATE_PREFLOP_BIG_BLIND = 0xF1
};


