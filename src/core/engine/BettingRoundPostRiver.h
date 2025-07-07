// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <iostream>
#include "BettingRound.h"

namespace pkt::core
{
class IHand;

class BettingRoundPostRiver : public BettingRound
{
  public:
    BettingRoundPostRiver(const GameEvents&, IHand*, int, int);
    ~BettingRoundPostRiver();

    void run();

  private:
    int myHighestCardsValue{0};
};

} // namespace pkt::core
