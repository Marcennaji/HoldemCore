
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
    IBotStrategy() : myPreflopRangeCalculator(std::make_unique<PreflopRangeCalculator>()) {}

    virtual bool preflopShouldCall(CurrentHandContext&) = 0;
    virtual bool flopShouldCall(CurrentHandContext&) = 0;
    virtual bool turnShouldCall(CurrentHandContext&) = 0;
    virtual bool riverShouldCall(CurrentHandContext&) = 0;

    virtual int preflopShouldRaise(CurrentHandContext&) = 0;
    virtual int flopShouldRaise(CurrentHandContext&) = 0;
    virtual int turnShouldRaise(CurrentHandContext&) = 0;
    virtual int riverShouldRaise(CurrentHandContext&) = 0;

    virtual int flopShouldBet(CurrentHandContext&) = 0;
    virtual int turnShouldBet(CurrentHandContext&) = 0;
    virtual int riverShouldBet(CurrentHandContext&) = 0;

    const std::string& getStrategyName() const { return myStrategyName; }
    void setStrategyName(const std::string& name) { myStrategyName = name; }
    const std::unique_ptr<PreflopRangeCalculator>& getPreflopRangeCalculator() const
    {
        return myPreflopRangeCalculator;
    }

    virtual ~IBotStrategy() = default;

  protected:
    void initializeRanges(const int utgHeadsUpRange, const int utgFullTableRange);

    int computePreflopRaiseAmount(CurrentHandContext&);
    bool shouldPotControl(CurrentHandContext&);

    bool myShouldCall = false;
    bool myShouldRaise = false;
    std::string myStrategyName;

  private:
    std::unique_ptr<PreflopRangeCalculator> myPreflopRangeCalculator;
};
} // namespace pkt::core::player
