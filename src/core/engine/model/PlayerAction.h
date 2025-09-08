
// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

namespace pkt::core
{

enum class ActionType
{
    None = 0,
    Fold,
    Check,
    Call,
    Bet,
    Raise,
    Allin,
    PostBigBlind,
    PostSmallBlind
};

struct PlayerAction
{
    int playerId = -1;
    ActionType type = ActionType::None;
    int amount = 0;
};

inline const char* playerActionToString(ActionType action)
{
    switch (action)
    {
    case ActionType::None:
        return "None";
    case ActionType::Fold:
        return "Fold";
    case ActionType::Check:
        return "Check";
    case ActionType::Call:
        return "Call";
    case ActionType::Bet:
        return "Bet";
    case ActionType::Raise:
        return "Raise";
    case ActionType::Allin:
        return "All-in";
    case ActionType::PostBigBlind:
        return "Post Big Blind";
    case ActionType::PostSmallBlind:
        return "Post Small Blind";
    default:
        return "Unknown Action";
    }
}

} // namespace pkt::core
