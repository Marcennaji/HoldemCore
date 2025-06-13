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

#define MIN_HANDS_STATISTICS_ACCURATE 30
