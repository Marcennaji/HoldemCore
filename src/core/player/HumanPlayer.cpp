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
#include "HumanPlayer.h"
#include <core/player/strategy/CurrentHandContext.h>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
namespace pkt::core::player
{

using namespace std;

HumanPlayer::HumanPlayer(GameEvents* events, ILogger* logger, IHandAuditStore* ha, IPlayersStatisticsStore* ps, int id,
                         std::string name, int sC, bool aS, int mB)
    : Player(events, logger, ha, ps, id, name, sC, aS, mB)
{
}

HumanPlayer::~HumanPlayer()
{
}

} // namespace pkt::core::player
