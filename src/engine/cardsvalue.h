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

#ifndef CARDSVALUE_H
#define CARDSVALUE_H

#include "game_defs.h"
#include <engine/engine.h>

#include <iostream>
#include <vector>
#include <map>

class CardsValue
{
public:

	static int holeCardsClass(int, int);
	static int cardsValue(int*, int*);
	static std::string determineHandName(int myCardsValueInt, PlayerList activePlayerList);
	static std::list<std::string> translateCardsValueCode(int cardsValueCode);

	static int holeCardsToIntCode(int*);
	static int* intCodeToHoleCards(int);

	//static std::vector< std::vector<int> > calcCardsChance(GameState, int*, int*);
	//static int** showdown(GameState, int**, int);

	static const std::string CardStringValue[];
	static std::map<std::string,int> CardStringOrdering;

};

#endif
