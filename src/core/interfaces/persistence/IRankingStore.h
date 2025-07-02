// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/player/typedefs.h>
#include "core/engine/EngineDefs.h"
namespace pkt::core
{

class IRankingStore
{
  public:
    virtual ~IRankingStore() = default;

    virtual void updateRankingGameLosers(pkt::core::player::PlayerList) = 0;
    virtual void updateRankingGameWinner(pkt::core::player::PlayerList) = 0;
    virtual void updateRankingPlayedGames(pkt::core::player::PlayerList) = 0;
};
} // namespace pkt::core
