#pragma once

#include <core/player/typedefs.h>
#include "core/engine/EngineDefs.h"
namespace pkt::core
{

class IRankingStore
{
  public:
    virtual ~IRankingStore() = default;

    virtual void updateRankingGameLosers(pkt::core::player::PlayerList activePlayerList) = 0;
    virtual void updateRankingGameWinner(pkt::core::player::PlayerList activePlayerList) = 0;
    virtual void updateRankingPlayedGames(pkt::core::player::PlayerList activePlayerList) = 0;
};
} // namespace pkt::core
