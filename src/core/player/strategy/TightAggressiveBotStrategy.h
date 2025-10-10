// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#pragma once

#include "core/player/strategy/BotStrategy.h"
#include "core/player/strategy/BotStrategyBase.h"
#include "core/interfaces/Logger.h"
#include "core/interfaces/Randomizer.h"

namespace pkt::core::player
{
// Forward declaration
struct CurrentHandContext;

/**
 * @brief Implements a tight-aggressive playing style for bot players.
 * 
 * This strategy plays a selective range of hands (tight) but plays them aggressively
 * with betting and raising. It's considered one of the most profitable poker strategies,
 * focusing on strong hands and applying pressure when holding them.
 */
class TightAggressiveBotStrategy : public BotStrategyBase
{

  public:
    TightAggressiveBotStrategy(pkt::core::Logger& logger, pkt::core::Randomizer& randomizer);
    ~TightAggressiveBotStrategy();

  private:
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
