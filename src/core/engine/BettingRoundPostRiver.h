/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the MIT License                                     *


 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * MIT License for more details.                                             *
 *                                                                           *
 * You should have received a copy of the MIT License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/

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
