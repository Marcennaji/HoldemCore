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

#include "EngineDefs.h"

#include <iostream>
#include <map>
#include <memory>
#include <vector>

namespace pkt::core
{

typedef std::shared_ptr<std::list<std::shared_ptr<Player>>> PlayerList;

class CardsValue
{
  public:
    static int holeCardsClass(int, int);
    static int cardsValue(int*, int*);
    static std::string determineHandName(int myCardsValueInt, PlayerList activePlayerList);
    static std::list<std::string> translateCardsValueCode(int cardsValueCode);

    static int holeCardsToIntCode(int*);
    static int* intCodeToHoleCards(int);

    static const std::string CardStringValue[];
    static std::map<std::string, int> CardStringOrdering;
};

} // namespace pkt::core
