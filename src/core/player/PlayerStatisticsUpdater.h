// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once

#include <core/engine/EngineDefs.h>
#include <core/engine/model/GameState.h>
#include <core/player/PlayerStatistics.h>

#include <core/ports/PlayersStatisticsStore.h>

#include <array>
#include <memory>
#include <string>

namespace pkt::core::player
{
struct CurrentHandContext;

/**
 * @brief Manages updating and persisting player statistics throughout gameplay.
 * 
 * Handles loading player statistics from storage, updating them based on
 * actions and hand outcomes, and persisting the updated statistics back
 * to the storage system.
 */
class PlayerStatisticsUpdater
{
  public:
    explicit PlayerStatisticsUpdater(pkt::core::PlayersStatisticsStore& statisticsStore);

    void loadStatistics(const std::string& playerName);
    void resetPlayerStatistics();
    void updateStatistics(GameState state, const CurrentHandContext& ctx);

    // Return the statistics for the specified number of players
    const PlayerStatistics& getStatistics(const int nbPlayers) const;

  private:
    void updatePreflopStatistics(const CurrentHandContext& ctx);
    void updateFlopStatistics(const CurrentHandContext& ctx);
    void updateTurnStatistics(const CurrentHandContext& ctx);
    void updateRiverStatistics(const CurrentHandContext& ctx);
    
    pkt::core::PlayersStatisticsStore& getPlayersStatisticsStore() const;

    core::PlayersStatisticsStore& m_statisticsStore;

    std::array<pkt::core::player::PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1> m_statistics;
};

} // namespace pkt::core::player
