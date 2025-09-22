// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
/* GameState */

#pragma once

#include <string>

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
inline std::string gameStateToString(GameState state)
{
    switch (state)
    {
    case Preflop:
        return "Preflop";
    case Flop:
        return "Flop";
    case Turn:
        return "Turn";
    case River:
        return "River";
    case PostRiver:
        return "PostRiver";
    case None:
        return "None";
    default:
        return "Unknown";
    }
}
} // namespace pkt::core
