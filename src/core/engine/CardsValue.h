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

#include <core/player/typedefs.h>
#include <iostream>
#include <map>
#include <memory>
#include <vector>
#include "EngineDefs.h"

namespace pkt::core
{

class CardsValue
{
  public:
    /// Evaluates the strength of a 7-card poker hand.
    /// \param cards An array of 7 integers (0–51) representing cards.
    /// \return A hand strength score where higher means better.
    static int evaluateHand(const int* cards);

    static const std::string CardStringValue[];
    static std::map<std::string, int> CardStringOrdering;
};

} // namespace pkt::core
