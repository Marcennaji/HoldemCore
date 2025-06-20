
// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

namespace pkt::core
{

enum PlayerAction
{
    PlayerActionNone = 0,
    PlayerActionFold,
    PlayerActionCheck,
    PlayerActionCall,
    PlayerActionBet,
    PlayerActionRaise,
    PlayerActionAllin
};

} // namespace pkt::core
