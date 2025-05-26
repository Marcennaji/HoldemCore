
#pragma once

namespace pkt::core
{

enum PlayerAction
{
    PLAYER_ACTION_NONE = 0,
    PLAYER_ACTION_FOLD,
    PLAYER_ACTION_CHECK,
    PLAYER_ACTION_CALL,
    PLAYER_ACTION_BET,
    PLAYER_ACTION_RAISE,
    PLAYER_ACTION_ALLIN
};

enum PlayerActionCode
{
    ACTION_CODE_VALID = 0,
    ACTION_CODE_INVALID_STATE,
    ACTION_CODE_NOT_YOUR_TURN,
    ACTION_CODE_NOT_ALLOWED
};

enum PlayerActionLog
{
    LOG_ACTION_NONE = 0,
    LOG_ACTION_DEALER,       // starts as dealer
    LOG_ACTION_SMALL_BLIND,  // posts small blind
    LOG_ACTION_BIG_BLIND,    // posts big blind
    LOG_ACTION_FOLD,         // folds
    LOG_ACTION_CHECK,        // checks
    LOG_ACTION_CALL,         // calls
    LOG_ACTION_BET,          // bets
    LOG_ACTION_ALL_IN,       // is all in with
    LOG_ACTION_SHOW,         // shows
    LOG_ACTION_HAS,          // has
    LOG_ACTION_WIN,          // wins
    LOG_ACTION_WIN_SIDE_POT, // wins (side pot)
    LOG_ACTION_SIT_OUT,      // sits out
    LOG_ACTION_WIN_GAME,     // wins game
    LOG_ACTION_ADMIN,        // is game admin now
    LOG_ACTION_JOIN          // has joined the game
};
} // namespace pkt::core
