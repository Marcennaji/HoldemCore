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
struct CurrentHandContext;

/**
 * @brief Implements a maniac playing style for bot players.
 *
 * This strategy plays very aggressively with almost any hand, frequently betting
 * and raising regardless of hand strength. It creates maximum action and pressure
 * but is generally unprofitable and highly volatile.
 */
class ManiacBotStrategy : public BotStrategyBase
{

  public:
    ManiacBotStrategy(pkt::core::Logger& logger, pkt::core::Randomizer& randomizer);

    ~ManiacBotStrategy();
    virtual std::string getName() const override { return "Maniac"; }

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
