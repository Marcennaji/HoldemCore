#pragma once

#include "core/engine/EngineDefs.h"

namespace pkt::core
{

class IRankingStore
{
  public:
    virtual ~IRankingStore() = default;

    virtual void updateRankingGameLosers(PlayerList activePlayerList) = 0;
    virtual void updateRankingGameWinner(PlayerList activePlayerList) = 0;
    virtual void updateRankingPlayedGames(PlayerList activePlayerList) = 0;
};
} // namespace pkt::core
