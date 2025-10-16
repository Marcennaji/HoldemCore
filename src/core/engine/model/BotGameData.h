// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <map>
#include <string>

namespace pkt::core
{

/**
 * @brief Configuration for bot-only games with custom strategy distribution
 */
struct BotGameData
{
    BotGameData() = default;

    int startMoney{0};
    int firstSmallBlind{0};

    /**
     * @brief Map of strategy names to player counts
     *
     * Keys: Strategy names ("tight", "loose", "maniac", "ultratight")
     * Values: Number of players with that strategy
     *
     * Example: {{"tight", 2}, {"loose", 2}, {"maniac", 1}}
     */
    std::map<std::string, int> strategyDistribution;
};

} // namespace pkt::core
