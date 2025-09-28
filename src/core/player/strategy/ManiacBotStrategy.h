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
    virtual bool preflopCouldCall(const CurrentHandContext& ctx);
    virtual bool flopCouldCall(const CurrentHandContext& ctx);
    virtual bool turnCouldCall(const CurrentHandContext& ctx);
    virtual bool riverCouldCall(const CurrentHandContext& ctx);

    virtual int preflopCouldRaise(const CurrentHandContext& ctx);
    virtual int flopCouldRaise(const CurrentHandContext& ctx);
    virtual int turnCouldRaise(const CurrentHandContext& ctx);
    virtual int riverCouldRaise(const CurrentHandContext& ctx);

    virtual int flopCouldBet(const CurrentHandContext& ctx);
    virtual int turnCouldBet(const CurrentHandContext& ctx);
    virtual int riverCouldBet(const CurrentHandContext& ctx);
};

} // namespace pkt::core::player
