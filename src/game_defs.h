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
#ifndef GAME_DEFS_H
#define GAME_DEFS_H

#define MIN_NUMBER_OF_PLAYERS		2
#define MAX_NUMBER_OF_PLAYERS		10

#define DEBUG_MODE			0
#define SQLITE_LOG          1
#define SQLITE_LOG_ALL      0

#define POKERTRAINING_VERSION_MAJOR	0
#define POKERTRAINING_VERSION_MINOR	1
#define POKERTRAINING_VERSION			((POKERTRAINING_VERSION_MAJOR << 8) | POKERTRAINING_VERSION_MINOR)

#define POKERTRAINING_BETA_REVISION	0
#define POKERTRAINING_BETA_RELEASE_STRING	 "1.5.0"

#define SQLITE_LOG_VERSION		1

#define GAME_START_CASH 100
#define GAME_NUMBER_OF_PLAYERS 6
#define GAME_START_SBLIND 5
#define GAME_SPEED 8
#define MIN_HANDS_STATISTICS_ACCURATE 30

enum GameState {
	GAME_STATE_PREFLOP = 0,
	GAME_STATE_FLOP,
	GAME_STATE_TURN,
	GAME_STATE_RIVER,
	GAME_STATE_POST_RIVER,
	GAME_STATE_PREFLOP_SMALL_BLIND = 0xF0,
	GAME_STATE_PREFLOP_BIG_BLIND = 0xF1
};

enum PlayerAction {
	PLAYER_ACTION_NONE = 0,
	PLAYER_ACTION_FOLD,
	PLAYER_ACTION_CHECK,
	PLAYER_ACTION_CALL,
	PLAYER_ACTION_BET,
	PLAYER_ACTION_RAISE,
	PLAYER_ACTION_ALLIN
};

enum PlayerActionCode {
	ACTION_CODE_VALID = 0,
	ACTION_CODE_INVALID_STATE,
	ACTION_CODE_NOT_YOUR_TURN,
	ACTION_CODE_NOT_ALLOWED
};

enum PlayerActionLog {
	LOG_ACTION_NONE = 0,
	LOG_ACTION_DEALER,      // starts as dealer
	LOG_ACTION_SMALL_BLIND, // posts small blind
	LOG_ACTION_BIG_BLIND,   // posts big blind
	LOG_ACTION_FOLD,        // folds
	LOG_ACTION_CHECK,       // checks
	LOG_ACTION_CALL,        // calls
	LOG_ACTION_BET,         // bets
	LOG_ACTION_ALL_IN,      // is all in with
	LOG_ACTION_SHOW,        // shows
	LOG_ACTION_HAS,         // has
	LOG_ACTION_WIN,         // wins
	LOG_ACTION_WIN_SIDE_POT,// wins (side pot)
	LOG_ACTION_SIT_OUT,     // sits out
	LOG_ACTION_WIN_GAME,    // wins game
	LOG_ACTION_ADMIN,		// is game admin now
	LOG_ACTION_JOIN			// has joined the game
};


enum Button {
	BUTTON_NONE = 0,
	BUTTON_DEALER,
	BUTTON_SMALL_BLIND,
	BUTTON_BIG_BLIND
};

enum PlayerPosition{
    UNKNOWN=0,
    SB=1,
    BB=2,
    UTG=3,
    UTG_PLUS_ONE=4,
    UTG_PLUS_TWO=5,
    MIDDLE=6,
    MIDDLE_PLUS_ONE=7,
    LATE=8,
    CUTOFF=9,
    BUTTON=10
};

#endif
