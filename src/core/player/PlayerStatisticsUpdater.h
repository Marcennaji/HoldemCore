#pragma once

#include <core/engine/EngineDefs.h>
#include <core/engine/model/GameState.h>
#include <core/player/PlayerStatistics.h>
#include <core/services/ServiceContainer.h>
#include <core/interfaces/HasPlayersStatisticsStore.h>

#include <array>
#include <memory>
#include <string>

namespace pkt::core::player
{
struct CurrentHandContext;

class PlayerStatisticsUpdater
{
  public:
    PlayerStatisticsUpdater() {}
    explicit PlayerStatisticsUpdater(std::shared_ptr<pkt::core::ServiceContainer> serviceContainer);
    
    // ISP-compliant constructor
    explicit PlayerStatisticsUpdater(std::shared_ptr<pkt::core::HasPlayersStatisticsStore> statisticsStore);

    void loadStatistics(const std::string& playerName);
    void resetPlayerStatistics();
    void updateStatistics(GameState state, const CurrentHandContext& ctx);

    // Return the statistics for the specified number of players
    const PlayerStatistics& getStatistics(const int nbPlayers) const;

  private:
    void ensureServicesInitialized() const;
    void updatePreflopStatistics(const CurrentHandContext& ctx);
    void updateFlopStatistics(const CurrentHandContext& ctx);
    void updateTurnStatistics(const CurrentHandContext& ctx);
    void updateRiverStatistics(const CurrentHandContext& ctx);
    
    // ISP-compliant helper methods
    pkt::core::PlayersStatisticsStore& getPlayersStatisticsStore() const;

    // Legacy service container (for backward compatibility)
    mutable std::shared_ptr<pkt::core::ServiceContainer> m_services;
    
    // ISP-compliant focused interface
    std::shared_ptr<pkt::core::HasPlayersStatisticsStore> m_statisticsStore;

    std::array<pkt::core::player::PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1> m_statistics;
};

} // namespace pkt::core::player
