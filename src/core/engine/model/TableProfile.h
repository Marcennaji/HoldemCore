// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
/* Game data. */

#pragma once

namespace pkt::core
{
enum TableProfile
{
    RandomOpponents = 1,
    TightAgressiveOpponents,
    LargeAgressiveOpponents,
    ManiacOpponents
};
}
