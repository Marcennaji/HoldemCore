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
    HumanPlayer(const GameEvents&, int id, int sC, bool aS, int mB);

    ~HumanPlayer();

    static std::string getName() { return "You"; }

    bool isBot() const override { return false; }
};

} // namespace pkt::core::player
