// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "BettingRound.h"

#include <iostream>

namespace pkt::core
{

class IHand;
class BettingRoundPreflop : public BettingRound
{

  public:
    BettingRoundPreflop(GameEvents*, ILogger* logger, IHand*, unsigned, int);
    ~BettingRoundPreflop();

    void run();

  private:
    // 	PlayerListIterator bigBlindPositionIt; // iterator for activePlayerList
    // 	unsigned bigBlindPositionId;
};
} // namespace pkt::core
