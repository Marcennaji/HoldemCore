// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
/* Game data. */

#pragma once

namespace pkt::core
{
enum GameState
{
    Preflop,
    Flop,
    Turn,
    River,
    PostRiver,
    None
};
}
