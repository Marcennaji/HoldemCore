// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <string>

#include <core/player/typedefs.h>
#include "core/engine/EngineDefs.h"
#include "core/player/PlayerStatistics.h"

namespace pkt::core
{

/**
 * @brief Abstract interface for persisting player statistics data.
 *
 * Defines the contract for loading and saving player statistics,
 * enabling different storage implementations (database, file system, etc.)
 * to be used for persistent player data management.
 *
 * Note: Statistics are stored per strategy type, not per individual player.
 * All players using the same strategy will share the same statistics record.
 */
class PlayersStatisticsStore
{
  public:
    virtual ~PlayersStatisticsStore() = default;

    virtual std::array<pkt::core::player::PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1>
    loadPlayerStatistics(const std::string& strategyName) = 0;

    virtual void savePlayersStatistics(pkt::core::player::PlayerList) = 0;
};
} //  namespace pkt::core
