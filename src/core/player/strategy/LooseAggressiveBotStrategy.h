// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/player/strategy/BotStrategy.h"
#include "core/player/strategy/BotStrategyBase.h"
#include "core/ports/Logger.h"
#include "core/ports/Randomizer.h"

namespace pkt::core::player
{
// Forward declaration
struct CurrentHandContext;

/**
 * @brief Implements a loose-aggressive playing style for bot players.
 *
 * This strategy plays a wider range of hands (loose) and plays them aggressively
 * with frequent betting and raising. It applies pressure across many situations
 * but can be more volatile than tight-aggressive play.
 */
class LooseAggressiveBotStrategy : public BotStrategyBase
{

  public:
    LooseAggressiveBotStrategy(pkt::core::Logger& logger, pkt::core::Randomizer& randomizer);
    ~LooseAggressiveBotStrategy();

    virtual std::string getName() const override { return "LooseAggressiveBot"; }

  protected:
    virtual bool preflopCouldCall(const CurrentHandContext& ctx) override;
    virtual bool flopCouldCall(const CurrentHandContext& ctx) override;
    virtual bool turnCouldCall(const CurrentHandContext& ctx) override;
    virtual bool riverCouldCall(const CurrentHandContext& ctx) override;

    virtual int preflopCouldRaise(const CurrentHandContext& ctx) override;
    virtual int flopCouldRaise(const CurrentHandContext& ctx) override;
    virtual int turnCouldRaise(const CurrentHandContext& ctx) override;
    virtual int riverCouldRaise(const CurrentHandContext& ctx) override;

    virtual int flopCouldBet(const CurrentHandContext& ctx) override;
    virtual int turnCouldBet(const CurrentHandContext& ctx) override;
    virtual int riverCouldBet(const CurrentHandContext& ctx) override;
};

} // namespace pkt::core::player
