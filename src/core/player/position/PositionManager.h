// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/engine/model/PlayerPosition.h"
#include "core/player/typedefs.h"

namespace pkt::core::player::position
{

/**
 * @brief Manages player positioning and position-related calculations
 *
 * This module handles position calculations, distance measurements,
 * and position-based advantages in poker gameplay.
 */
class PositionManager
{
  public:
    /**
     * @brief Check if a player has positional advantage
     *
     * @param position Player's position
     * @param actingPlayers List of players still acting
     * @return true if player has position (acts last), false otherwise
     */
    static bool hasPosition(PlayerPosition position, const PlayerList& actingPlayers);

    /**
     * @brief Calculate circular distance between two players
     *
     * @param fromId Starting player ID
     * @param toId Target player ID
     * @param players List of all players
     * @return Number of positions between players (circular)
     */
    static int playerDistanceCircularOffset(int fromId, int toId, const PlayerList& players);

    /**
     * @brief Convert seat offset to player position
     *
     * @param offset Seat offset from dealer
     * @param nbPlayers Total number of players
     * @return Corresponding PlayerPosition enum value
     */
    static PlayerPosition computePlayerPositionFromOffset(int offset, int nbPlayers);
};

} // namespace pkt::core::player::position
