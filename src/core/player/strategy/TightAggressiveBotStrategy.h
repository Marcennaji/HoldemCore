/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU Affero General Public License as            *
 * published by the Free Software Foundation, either version 3 of the        *
 * License, or (at your option) any later version.                           *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU Affero General Public License for more details.                       *
 *                                                                           *
 * You should have received a copy of the GNU Affero General Public License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/
#pragma once

#include "core/player/strategy/IBotStrategy.h"
namespace pkt::core::player
{
// Forward declaration
struct CurrentHandContext;
class TightAggressiveBotStrategy : public IBotStrategy
{

  public:
    TightAggressiveBotStrategy();

    ~TightAggressiveBotStrategy();

    bool preflopShouldCall(CurrentHandContext& context, bool deterministic = false) override;
    bool flopShouldCall(CurrentHandContext& context, bool deterministic = false) override;
    bool turnShouldCall(CurrentHandContext& context, bool deterministic = false) override;
    bool riverShouldCall(CurrentHandContext& context, bool deterministic = false) override;

    int preflopShouldRaise(CurrentHandContext& context, bool deterministic = false) override;
    int flopShouldRaise(CurrentHandContext& context, bool deterministic = false) override;
    int turnShouldRaise(CurrentHandContext& context, bool deterministic = false) override;
    int riverShouldRaise(CurrentHandContext& context, bool deterministic = false) override;

    int flopShouldBet(CurrentHandContext& context, bool deterministic = false) override;
    int turnShouldBet(CurrentHandContext& context, bool deterministic = false) override;
    int riverShouldBet(CurrentHandContext& context, bool deterministic = false) override;
};

} // namespace pkt::core::player
