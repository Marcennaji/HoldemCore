
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

inline const char* playerActionToString(PlayerAction action)
{
    switch (action)
    {
    case PlayerActionNone:
        return "None";
    case PlayerActionFold:
        return "Fold";
    case PlayerActionCheck:
        return "Check";
    case PlayerActionCall:
        return "Call";
    case PlayerActionBet:
        return "Bet";
    case PlayerActionRaise:
        return "Raise";
    case PlayerActionAllin:
        return "All-in";
    default:
        return "Unknown Action";
    }
}

} // namespace pkt::core
