// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <string>

namespace pkt::ui::qtwidgets
{

/**
 * @brief Lightweight DTO for player display information.
 *
 * This Data Transfer Object carries only the information needed by the UI
 * to display a player, avoiding the need to pass the entire Player facade
 * across architectural boundaries. This respects both the Interface Segregation
 * Principle and hexagonal architecture by minimizing coupling.
 */
struct PlayerDisplayInfo
{
    int playerId;
    std::string playerName;
    std::string strategyName; // Empty for human players or "None" if not set
    int chips;

    PlayerDisplayInfo() : playerId(-1), playerName(""), strategyName(""), chips(0) {}

    PlayerDisplayInfo(int id, const std::string& name, const std::string& strategy, int cash)
        : playerId(id), playerName(name), strategyName(strategy), chips(cash)
    {
    }
};

} // namespace pkt::ui::qtwidgets
