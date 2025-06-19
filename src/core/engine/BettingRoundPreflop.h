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
    BettingRoundPreflop(const GameEvents&, IHand*, unsigned, int);
    ~BettingRoundPreflop();

    void run();
};
} // namespace pkt::core
