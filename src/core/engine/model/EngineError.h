// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

namespace pkt::core
{

enum class EngineError
{
    SEAT_NOT_FOUND = 10001,
    ACTIVE_PLAYER_NOT_FOUND = 10002,
    RUNNING_PLAYER_NOT_FOUND = 10003,
    DEALER_NOT_FOUND = 10004,
    CURRENT_PLAYER_NOT_FOUND = 10005,
    NEXT_DEALER_NOT_FOUND = 10010,
    NEXT_ACTIVE_PLAYER_NOT_FOUND = 10011,
    FORMER_RUNNING_PLAYER_NOT_FOUND = 10012,
    PLAYER_ACTION_ERROR = 10030
};
} // namespace pkt::core
