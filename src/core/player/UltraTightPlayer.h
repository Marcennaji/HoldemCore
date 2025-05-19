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
#ifndef PLAYING_STYLE_ULTRA_TIGHT_H
#define PLAYING_STYLE_ULTRA_TIGHT_H

#include "Player.h"

class UltraTightPlayer : public Player
{

  public:
    UltraTightPlayer(GameEvents*, IHandAuditStore*, IPlayersStatisticsStore*, int id, PlayerType type, std::string name,
                     int sC, bool aS, bool sotS, int mB);

    ~UltraTightPlayer();

  protected:
    bool preflopShouldCall();
    bool flopShouldCall();
    bool turnShouldCall();
    bool riverShouldCall();

    bool preflopShouldRaise();
    bool flopShouldRaise();
    bool turnShouldRaise();
    bool riverShouldRaise();

    bool flopShouldBet();
    bool turnShouldBet();
    bool riverShouldBet();
};

#endif
