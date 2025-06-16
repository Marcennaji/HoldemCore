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

#include "Player.h"
namespace pkt::core::player
{
class HumanPlayer : public Player
{

  public:
    HumanPlayer(GameEvents*, ILogger*, IHandAuditStore*, IPlayersStatisticsStore*, int id, std::string name, int sC,
                bool aS, int mB);

    ~HumanPlayer();

    virtual std::string getStrategyName() const override { return "HumanPlayer"; }
};

} // namespace pkt::core::player
