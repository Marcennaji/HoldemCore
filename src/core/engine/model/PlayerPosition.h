// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once
namespace pkt::core
{

enum PlayerPosition
{
    UNKNOWN = 0,
    SB = 1,
    BB = 2,
    UTG = 3,
    UTG_PLUS_ONE = 4,
    UTG_PLUS_TWO = 5,
    MIDDLE = 6,
    MIDDLE_PLUS_ONE = 7,
    LATE = 8,
    CUTOFF = 9,
    BUTTON = 10
};

} // namespace pkt::core
