// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

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
    /// \param hand A string representing the 7-card poker hand (5 board + 2 hole).
    /// \return A hand strength score where higher means better.
    static unsigned int evaluateHand(const char* hand);

    static const std::string CardStringValue[];
    static std::map<std::string, int> CardStringOrdering;
};

} // namespace pkt::core
