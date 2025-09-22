// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

namespace pkt::core
{

enum class EngineError
{
    /* deprecated */
    ActivePlayerNotFound,
    NextActivePlayerNotFound,
    /* used by FSM */
    SeatNotFound,
    ActingPlayerNotFound,
    DealerNotFound,
    NextDealerNotFound,
    PlayerActionError,
    MissingParameter
};
} // namespace pkt::core
