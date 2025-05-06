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

#ifndef BEROPREFLOP_H
#define BEROPREFLOP_H

#include <iostream>
#include "BettingRound.h"

class HandInterface;

class BettingRoundPreflop : public BettingRound
{

public:
	BettingRoundPreflop(HandInterface*, unsigned, int);
	~BettingRoundPreflop();

	void run();

private:
// 	PlayerListIterator bigBlindPositionIt; // iterator for activePlayerList
// 	unsigned bigBlindPositionId;
};

#endif
