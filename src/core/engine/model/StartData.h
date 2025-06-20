// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

namespace pkt::core
{

struct StartData
{
    StartData() {}
    unsigned startDealerPlayerId{0};
    int numberOfPlayers{0};
};

} // namespace pkt::core
