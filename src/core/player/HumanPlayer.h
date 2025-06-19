// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "Player.h"
namespace pkt::core::player
{
class HumanPlayer : public Player
{

  public:
    HumanPlayer(const GameEvents&, ILogger*, IHandAuditStore*, IPlayersStatisticsStore*, int id, std::string name,
                int sC, bool aS, int mB);

    ~HumanPlayer();

    virtual std::string getStrategyName() const override { return "HumanPlayer"; }
};

} // namespace pkt::core::player
