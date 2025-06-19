// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/interfaces/persistence/IRankingStore.h>
namespace pkt::core
{

class NullRankingStore : public IRankingStore
{
  public:
    virtual ~NullRankingStore() = default;

    virtual void updateRankingGameLosers(pkt::core::player::PlayerList activePlayerList) override {}
    virtual void updateRankingGameWinner(pkt::core::player::PlayerList activePlayerList) override {}
    virtual void updateRankingPlayedGames(pkt::core::player::PlayerList activePlayerList) override {}
};
} // namespace pkt::core
