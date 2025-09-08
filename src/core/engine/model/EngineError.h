// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

namespace pkt::core
{

enum class EngineError
{
    SeatNotFound = 10001,
    ActivePlayerNotFound = 10002,
    ActingPlayerNotFound = 10003,
    DealerNotFound = 10004,
    CurrentPlayerNotFound = 10005,
    NextDealerNotFound = 10010,
    NextActivePlayerNotFound = 10011,
    FormerActingPlayerNotFound = 10012,
    PlayerActionError = 10030
};
} // namespace pkt::core
