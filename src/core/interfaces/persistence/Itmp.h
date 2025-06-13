#pragma once

#include <string>

#include <core/player/typedefs.h>
#include "core/engine/EngineDefs.h"
#include "core/engine/model/PlayerStatistics.h"

namespace pkt::core
{

class IPlayersStatisticsStore
{
  public:
    virtual ~IPlayersStatisticsStore() = default;

    virtual std::array<pkt::core::PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1>
    getPlayerStatistics(const std::string& playerName) = 0;

    virtual void updatePlayersStatistics(pkt::core::player::PlayerList activePlayerList) = 0;
};
} //  namespace pkt::core
