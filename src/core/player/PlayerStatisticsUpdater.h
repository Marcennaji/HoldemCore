#pragma once

#include <core/engine/EngineDefs.h>
#include <core/player/PlayerStatistics.h>

#include <array>
#include <string>

namespace pkt::core::player
{
class CurrentHandContext;

class PlayerStatisticsUpdater
{
  public:
    PlayerStatisticsUpdater() {}

    void loadStatistics(const std::string& playerName);
    void resetPlayerStatistics();

    // Return the statistics for the specified number of players
    const PlayerStatistics& getStatistics(const int nbPlayers) const;
    void updatePreflopStatistics(const CurrentHandContext& ctx, const ActionType& action);
    void updateFlopStatistics(const CurrentHandContext& ctx, const ActionType& action);
    void updateTurnStatistics(const CurrentHandContext& ctx, const ActionType& action);
    void updateRiverStatistics(const CurrentHandContext& ctx, const ActionType& action);

  private:
    std::array<pkt::core::player::PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1> myStatistics;
};

} // namespace pkt::core::player
