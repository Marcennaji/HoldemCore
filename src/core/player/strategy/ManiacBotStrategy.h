// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#pragma once

#include "core/player/strategy/IBotStrategy.h"
namespace pkt::core::player
{
// Forward declaration
struct CurrentHandContext;
class ManiacBotStrategy : public IBotStrategy
{

  public:
    ManiacBotStrategy();

    ~ManiacBotStrategy();

    bool preflopShouldCall(CurrentHandContext& ctx) override;
    bool flopShouldCall(CurrentHandContext& ctx) override;
    bool turnShouldCall(CurrentHandContext& ctx) override;
    bool riverShouldCall(CurrentHandContext& ctx) override;

    int preflopShouldRaise(CurrentHandContext& ctx) override;
    int flopShouldRaise(CurrentHandContext& ctx) override;
    int turnShouldRaise(CurrentHandContext& ctx) override;
    int riverShouldRaise(CurrentHandContext& ctx) override;

    int flopShouldBet(CurrentHandContext& ctx) override;
    int turnShouldBet(CurrentHandContext& ctx) override;
    int riverShouldBet(CurrentHandContext& ctx) override;
};

} // namespace pkt::core::player
