// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/Logger.h"

namespace pkt::core
{

class Hand;
struct PlayerAction;

namespace player
{
class Player;
}

/**
 * @brief Applies player actions to the game state during a poker hand.
 * 
 * This utility class handles the application of various player actions
 * (call, raise, bet, fold, etc.) to the current hand state, updating
 * player balances, pot amounts, and game history accordingly.
 */
class ActionApplier
{
  public:
    static void apply(Hand& hand, const PlayerAction& action, pkt::core::Logger& logger);

  private:
    static void applyCallAction(Hand& hand, player::Player& player, PlayerAction& actionForHistory, int currentHighest,
                                int playerBet);
    static void applyRaiseAction(Hand& hand, player::Player& player, PlayerAction& actionForHistory,
                                 const PlayerAction& originalAction, int playerBet);
    static void applyBetAction(Hand& hand, player::Player& player, const PlayerAction& action);
    static void applyAllinAction(Hand& hand, player::Player& player, PlayerAction& actionForHistory,
                                 int currentHighest);

    static void updateBetAndPot(Hand& hand, player::Player& player, int amount);
    static void setLastRaiserForCurrentRound(Hand& hand, unsigned playerId);
    static void finalizeAction(Hand& hand, player::Player& player, const PlayerAction& actionForHistory, pkt::core::Logger& logger);
};

} // namespace pkt::core
