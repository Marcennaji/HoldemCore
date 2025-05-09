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
#ifndef HUMAN_PLAYER_H
#define HUMAN_PLAYER_H

#include "player.h"


class HumanPlayer : public Player{

public: 

	HumanPlayer(Log *, int id, PlayerType type, std::string name, int sC, bool aS, bool sotS, int mB);

	~HumanPlayer();

	const SimResults getHandSimulation() const;


protected :

	bool preflopShouldCall(){return false;}
	bool flopShouldCall(){return false;}
	bool turnShouldCall(){return false;}
	bool riverShouldCall(){return false;}

	bool preflopShouldRaise(){return false;}
	bool flopShouldRaise(){return false;}
	bool turnShouldRaise(){return false;}
	bool riverShouldRaise(){return false;}

	bool flopShouldBet(){return false;}
	bool turnShouldBet(){return false;}
	bool riverShouldBet(){return false;}
};

#endif
