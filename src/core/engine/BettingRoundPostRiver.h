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
    BettingRoundPostRiver(GameEvents*, ILogger* logger, IHand*, int, int);
    ~BettingRoundPostRiver();

    void setHighestCardsValue(int theValue);
    int getHighestCardsValue() const;
    void run();
    void postRiverRun();

  private:
    int highestCardsValue;
};

} // namespace pkt::core
