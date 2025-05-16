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

#include "BettingRoundRiver.h"
#include "core/interfaces/IHand.h"

#include <iostream>

using namespace std;

BettingRoundRiver::BettingRoundRiver(ILogger* logger, IHand* hi, unsigned dP, int sB)
    : BettingRound(logger, hi, dP, sB, GAME_STATE_RIVER)
{
}

BettingRoundRiver::~BettingRoundRiver()
{
}
