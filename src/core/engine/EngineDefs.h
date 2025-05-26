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

#include <list>
#include <memory>

namespace pkt::core
{
class Player;

typedef std::shared_ptr<std::list<std::shared_ptr<Player>>> PlayerList;
typedef std::list<std::shared_ptr<Player>>::iterator PlayerListIterator;
typedef std::list<std::shared_ptr<Player>>::const_iterator PlayerListConstIterator;
} // namespace pkt::core

#define MIN_NUMBER_OF_PLAYERS 2
#define MAX_NUMBER_OF_PLAYERS 10

#define DEBUG_MODE 0
#define SQLITE_LOG 1
#define SQLITE_LOG_ALL 0

#define POKERTRAINING_VERSION_MAJOR 0
#define POKERTRAINING_VERSION_MINOR 1
#define POKERTRAINING_VERSION ((POKERTRAINING_VERSION_MAJOR << 8) | POKERTRAINING_VERSION_MINOR)

#define POKERTRAINING_BETA_REVISION 0
#define POKERTRAINING_BETA_RELEASE_STRING "1.5.0"

#define SQLITE_LOG_VERSION 1

#define GAME_START_CASH 100
#define GAME_NUMBER_OF_PLAYERS 6
#define GAME_START_SBLIND 5
#define GAME_SPEED 8
#define MIN_HANDS_STATISTICS_ACCURATE 30
