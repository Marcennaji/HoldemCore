// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <string>

#include <core/player/typedefs.h>
#include "core/engine/EngineDefs.h"
#include "core/player/PlayerStatistics.h"

namespace pkt::core
{

class PlayersStatisticsStore
{
  public:
    virtual ~PlayersStatisticsStore() = default;

    virtual std::array<pkt::core::player::PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1>
    loadPlayerStatistics(const std::string& playerName) = 0;

    virtual void savePlayersStatistics(pkt::core::player::PlayerFsmList) = 0;
};
} //  namespace pkt::core
