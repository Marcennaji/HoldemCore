// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "HandCalculator.h"
#include "core/interfaces/Logger.h"
#include <algorithm>

namespace pkt::core
{

HandCalculator::HandCalculator(Logger& logger) : m_logger(&logger)
{
}

float HandCalculator::calculateM(int cash, int smallBlind) const
{
    int blinds = smallBlind + (smallBlind * 2); // small blind + big blind
    if (blinds > 0 && cash > 0)
    {
        return static_cast<float>(cash) / blinds;
    }
    else
    {
        return 0.0f;
    }
}

int HandCalculator::calculatePotOdd(int playerCash, int playerSet, int pot, int sets, 
                                   int roundHighestSet, Logger& logger) const
{
    const int highestBetAmount = std::min(playerCash, roundHighestSet);

    int totalPot = pot + sets;

    if (totalPot == 0)
    { 
        // This shouldn't happen, but handle gracefully
        logger.error("Pot calculation error: Pot = " + std::to_string(pot) + 
                    " + Sets = " + std::to_string(sets) + 
                    " = " + std::to_string(totalPot));
        return 0;
    }

    int odd = (highestBetAmount - playerSet) * 100 / totalPot;
    if (odd < 0)
    {
        odd = -odd; // Handle case where playerSet > highestBetAmount
    }

    return odd;
}

} // namespace pkt::core