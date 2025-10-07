#pragma once

#include <core/engine/EngineDefs.h>
#include <core/engine/model/GameState.h>
#include <core/player/PlayerStatistics.h>

#include <core/interfaces/persistence/PlayersStatisticsStore.h>

#include <array>
#include <memory>
#include <string>

namespace pkt::core::player
{
struct CurrentHandContext;

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

    pkt::core::PlayersStatisticsStore* m_statisticsStore;

    std::array<pkt::core::player::PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1> m_statistics;
};

} // namespace pkt::core::player
