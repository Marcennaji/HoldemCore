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

#include "GameMode.h"
#include "GameData.h"

#include <list>
#include <string>

struct GameInfo {
	GameInfo() : mode(GAME_MODE_CREATED), adminPlayerId(0), isPasswordProtected(false) {}
	std::string name;
	GameData data;
	GameMode mode;
	unsigned adminPlayerId;
	std::list<unsigned> players;
	bool isPasswordProtected;
};


