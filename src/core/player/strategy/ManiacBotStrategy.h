// HoldemCore — Texas Hold'em simulator
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
    ManiacBotStrategy(std::shared_ptr<pkt::core::ServiceContainer> serviceContainer);
    ~ManiacBotStrategy();

  protected:
    virtual bool preflopShouldCall(const CurrentHandContext& ctx);
    virtual bool flopShouldCall(const CurrentHandContext& ctx);
    virtual bool turnShouldCall(const CurrentHandContext& ctx);
    virtual bool riverShouldCall(const CurrentHandContext& ctx);

    virtual int preflopShouldRaise(const CurrentHandContext& ctx);
    virtual int flopShouldRaise(const CurrentHandContext& ctx);
    virtual int turnShouldRaise(const CurrentHandContext& ctx);
    virtual int riverShouldRaise(const CurrentHandContext& ctx);

    virtual int flopShouldBet(const CurrentHandContext& ctx);
    virtual int turnShouldBet(const CurrentHandContext& ctx);
    virtual int riverShouldBet(const CurrentHandContext& ctx);
};

} // namespace pkt::core::player
