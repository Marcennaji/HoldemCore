// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <memory>
#include "typedefs.h"

namespace pkt::core
{
class Logger;
class ServiceContainer;
} // namespace pkt::core

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
     * @brief Find a Player by ID in a list
     *
     * @param list List of Player objects
     * @param id Player ID to search for
     * @return Shared pointer to Player if found, nullptr otherwise
     */
    static std::shared_ptr<Player> getPlayerById(const PlayerList& list, unsigned id);

    /**
     * @brief Get iterator to Player by ID
     *
     * @param list List of Player objects
     * @param id Player ID to search for
     * @return Iterator to the Player if found, end() otherwise
     */
    static PlayerListIterator getPlayerListIteratorById(PlayerList& list, unsigned id);

    /**
     * @brief Update acting players list by removing folded/all-in players
     *
     * @param actingPlayersList List to update (modified in place)
     */
    static void updateActingPlayersList(PlayerList& actingPlayersList);

    /**
     * @brief Update acting players list by removing folded/all-in players
     *
     * @param actingPlayersList List to update (modified in place)
     * @param services ServiceContainer for accessing services
     */
    static void updateActingPlayersList(PlayerList& actingPlayersList,
                                        std::shared_ptr<pkt::core::ServiceContainer> services);

    /**
     * @brief Update acting players list by removing folded/all-in players
     *
     * @param actingPlayersList List to update (modified in place)
     * @param logger Logger to use for verbose output
     */
    static void updateActingPlayersList(PlayerList& actingPlayersList, pkt::core::Logger& logger);
};

} // namespace pkt::core::player
