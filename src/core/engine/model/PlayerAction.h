
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

enum PlayerActionCode
{
    ActionCodeValid = 0,
    ActionCodeInvalidState,
    ActionCodeNotYourTurn,
    ActionCodeNotAllowed
};

enum PlayerActionLog
{
    LogActionNone = 0,
    LogActionDealer,     // starts as dealer
    LogActionSmallBlind, // posts small blind
    LogActionBigBlind,   // posts big blind
    LogActionFold,       // folds
    LogActionCheck,      // checks
    LogActionCall,       // calls
    LogActionBet,        // bets
    LogActionAllIn,      // is all in with
    LogActionShow,       // shows
    LogActionHas,        // has
    LogActionWin,        // wins
    LogActionWinSidePot, // wins (side pot)
    LogActionSitOut,     // sits out
    LogActionWinGame,    // wins game
    LogActionAdmin,      // is game admin now
    LogActionJoin        // has joined the game
};
} // namespace pkt::core
