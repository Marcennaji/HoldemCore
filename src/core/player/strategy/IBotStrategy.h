
// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/player/Player.h>
#include <core/player/strategy/PreflopRangeCalculator.h>

namespace pkt::core::player
{
// Forward declaration
struct CurrentHandContext;

class IBotStrategy
{
  public:
    virtual ~IBotStrategy() = default;

    PlayerAction decideAction(const CurrentHandContext& ctx)
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

    const std::string& getStrategyName() const { return myStrategyName; }
    const std::unique_ptr<PreflopRangeCalculator>& getPreflopRangeCalculator() const
    {
        return myPreflopRangeCalculator;
    }

  protected:
    IBotStrategy(const std::string& name)
        : myStrategyName(name), myPreflopRangeCalculator(std::make_unique<PreflopRangeCalculator>())
    {
    }

    void initializeRanges(const int utgHeadsUpRange, const int utgFullTableRange)
    {
        myPreflopRangeCalculator->initializeRanges(utgHeadsUpRange, utgFullTableRange);
    }

    // bots strategies implementers must override these methods
    virtual PlayerAction decidePreflop(const CurrentHandContext&) = 0;
    virtual PlayerAction decideFlop(const CurrentHandContext&) = 0;
    virtual PlayerAction decideTurn(const CurrentHandContext&) = 0;
    virtual PlayerAction decideRiver(const CurrentHandContext&) = 0;

    const std::string myStrategyName;
    std::unique_ptr<PreflopRangeCalculator> myPreflopRangeCalculator;
};

} // namespace pkt::core::player
