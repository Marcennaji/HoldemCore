// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <memory>
#include "typedefs.h"

namespace pkt::core::player
{

/**
 * @brief Utilities for managing player lists and player operations
 *
 * This module provides core functionality for finding, updating,
 * and managing player collections.
 */
class PlayerListUtils
{
  public:
    /**
     * @brief Find a PlayerFsm by ID in a list
     *
     * @param list List of PlayerFsm objects
     * @param id Player ID to search for
     * @return Shared pointer to PlayerFsm if found, nullptr otherwise
     */
    static std::shared_ptr<PlayerFsm> getPlayerFsmById(const PlayerFsmList& list, unsigned id);

    /**
     * @brief Get iterator to PlayerFsm by ID
     *
     * @param list List of PlayerFsm objects
     * @param id Player ID to search for
     * @return Iterator to the PlayerFsm if found, end() otherwise
     */
    static PlayerFsmListIterator getPlayerFsmListIteratorById(PlayerFsmList& list, unsigned id);

    /**
     * @brief Update acting players list by removing folded/all-in players
     *
     * @param actingPlayersList List to update (modified in place)
     */
    static void updateActingPlayersListFsm(PlayerFsmList& actingPlayersList);
};

} // namespace pkt::core::player
