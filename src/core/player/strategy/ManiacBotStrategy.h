// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#pragma once

#include "core/player/strategy/BotStrategy.h"
#include "core/player/strategy/BotStrategyBase.h"
namespace pkt::core::player
{
// Forward declaration
struct CurrentHandContext;
class ManiacBotStrategy : public BotStrategyBase
{

  public:
    ManiacBotStrategy();
    ~ManiacBotStrategy();

  protected:
    bool preflopShouldCall(const CurrentHandContext& ctx);
    bool flopShouldCall(const CurrentHandContext& ctx);
    bool turnShouldCall(const CurrentHandContext& ctx);
    bool riverShouldCall(const CurrentHandContext& ctx);

    int preflopShouldRaise(const CurrentHandContext& ctx);
    int flopShouldRaise(const CurrentHandContext& ctx);
    int turnShouldRaise(const CurrentHandContext& ctx);
    int riverShouldRaise(const CurrentHandContext& ctx);

    int flopShouldBet(const CurrentHandContext& ctx);
    int turnShouldBet(const CurrentHandContext& ctx);
    int riverShouldBet(const CurrentHandContext& ctx);
};

} // namespace pkt::core::player
