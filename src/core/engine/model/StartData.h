// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

namespace pkt::core
{

struct StartData
{
    StartData() : startDealerPlayerId(0), numberOfPlayers(0) {}
    unsigned startDealerPlayerId;
    int numberOfPlayers;
};

} // namespace pkt::core
