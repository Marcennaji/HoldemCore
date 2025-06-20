// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
/* Game data. */

#pragma once

namespace pkt::core
{
enum GameState
{
    GameStatePreflop = 0,
    GameStateFlop,
    GameStateTurn,
    GameStateRiver,
    GameStatePostRiver,
    GameStateNone
};
}
