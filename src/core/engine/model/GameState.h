// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
/* Game data. */

#pragma once

namespace pkt::core
{
enum GameState
{
    GAME_STATE_PREFLOP = 0,
    GAME_STATE_FLOP,
    GAME_STATE_TURN,
    GAME_STATE_RIVER,
    GAME_STATE_POST_RIVER,
    GAME_STATE_NONE
};
}
