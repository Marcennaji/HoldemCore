
// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "PlayerStrategy.h"
#include "core/player/strategy/CurrentHandContext.h"
#include "core/player/strategy/PreflopRangeCalculator.h"

namespace pkt::core::player
{
// Forward declaration
struct CurrentHandContext;

class BotStrategy : public PlayerStrategy
{
  public:
    virtual ~BotStrategy() = default;

    PlayerAction decideAction(const CurrentHandContext& ctx) override
    {
        switch (ctx.commonContext.gameState)
        {
        case Preflop:
            return decidePreflop(ctx);
        case Flop:
            return decideFlop(ctx);
        case Turn:
            return decideTurn(ctx);
        case River:
            return decideRiver(ctx);
        default:
            return {ctx.personalContext.id, ActionType::None, 0};
        }
    }

    const std::unique_ptr<PreflopRangeCalculator>& getPreflopRangeCalculator() const
    {
        return m_preflopRangeCalculator;
    }

  protected:

    void initializeRanges(const int utgHeadsUpRange, const int utgFullTableRange)
    {
        m_preflopRangeCalculator->initializeRanges(utgHeadsUpRange, utgFullTableRange);
    }

    // bots strategies implementers must override only these methods
    virtual PlayerAction decidePreflop(const CurrentHandContext&) = 0;
    virtual PlayerAction decideFlop(const CurrentHandContext&) = 0;
    virtual PlayerAction decideTurn(const CurrentHandContext&) = 0;
    virtual PlayerAction decideRiver(const CurrentHandContext&) = 0;

    std::unique_ptr<PreflopRangeCalculator> m_preflopRangeCalculator;
};

} // namespace pkt::core::player
