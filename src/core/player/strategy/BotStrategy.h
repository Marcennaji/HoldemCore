
// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/player/deprecated/Player.h>
#include <core/player/strategy/PreflopRangeCalculator.h>
#include "PlayerStrategy.h"

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
            return {ctx.perPlayerContext.myID, ActionType::None, 0};
        }
    }

    const std::unique_ptr<PreflopRangeCalculator>& getPreflopRangeCalculator() const
    {
        return myPreflopRangeCalculator;
    }

  protected:
    BotStrategy() : myPreflopRangeCalculator(std::make_unique<PreflopRangeCalculator>()) {}

    void initializeRanges(const int utgHeadsUpRange, const int utgFullTableRange)
    {
        myPreflopRangeCalculator->initializeRanges(utgHeadsUpRange, utgFullTableRange);
    }

    // bots strategies implementers must override these methods
    virtual PlayerAction decidePreflop(const CurrentHandContext&) = 0;
    virtual PlayerAction decideFlop(const CurrentHandContext&) = 0;
    virtual PlayerAction decideTurn(const CurrentHandContext&) = 0;
    virtual PlayerAction decideRiver(const CurrentHandContext&) = 0;

    std::unique_ptr<PreflopRangeCalculator> myPreflopRangeCalculator;
};

} // namespace pkt::core::player
