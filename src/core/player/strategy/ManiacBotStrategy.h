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

    bool preflopShouldCall(CurrentHandContext& context) override;
    bool flopShouldCall(CurrentHandContext& context) override;
    bool turnShouldCall(CurrentHandContext& context) override;
    bool riverShouldCall(CurrentHandContext& context) override;

    int preflopShouldRaise(CurrentHandContext& context) override;
    int flopShouldRaise(CurrentHandContext& context) override;
    int turnShouldRaise(CurrentHandContext& context) override;
    int riverShouldRaise(CurrentHandContext& context) override;

    int flopShouldBet(CurrentHandContext& context) override;
    int turnShouldBet(CurrentHandContext& context) override;
    int riverShouldBet(CurrentHandContext& context) override;
};

} // namespace pkt::core::player
