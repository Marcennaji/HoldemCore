// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

namespace pkt::core
{

struct StartData
{
    StartData() = default;
    
    // If startDealerPlayerId is UINT_MAX, the Session will randomly select a dealer
    static constexpr unsigned AUTO_SELECT_DEALER = UINT_MAX;
    
    unsigned startDealerPlayerId{AUTO_SELECT_DEALER};
    int numberOfPlayers{0};
};

} // namespace pkt::core
