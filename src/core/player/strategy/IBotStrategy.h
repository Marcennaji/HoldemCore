
#pragma once

#include <core/interfaces/ILogger.h>
#include <core/player/Player.h>
#include <core/player/strategy/PreflopRangeCalculator.h>

namespace pkt::core::player
{
// Forward declaration
struct CurrentHandContext;

class IBotStrategy
{
  public:
    IBotStrategy(ILogger* myLogger)
        : myLogger(myLogger), preflopRangeCalculator(std::make_unique<PreflopRangeCalculator>())
    {
    }

    virtual bool preflopShouldCall(CurrentHandContext&, bool deterministic = false) = 0;
    virtual bool flopShouldCall(CurrentHandContext&, bool deterministic = false) = 0;
    virtual bool turnShouldCall(CurrentHandContext&, bool deterministic = false) = 0;
    virtual bool riverShouldCall(CurrentHandContext&, bool deterministic = false) = 0;

    virtual int preflopShouldRaise(CurrentHandContext&, bool deterministic = false) = 0;
    virtual int flopShouldRaise(CurrentHandContext&, bool deterministic = false) = 0;
    virtual int turnShouldRaise(CurrentHandContext&, bool deterministic = false) = 0;
    virtual int riverShouldRaise(CurrentHandContext&, bool deterministic = false) = 0;

    virtual int flopShouldBet(CurrentHandContext&, bool deterministic = false) = 0;
    virtual int turnShouldBet(CurrentHandContext&, bool deterministic = false) = 0;
    virtual int riverShouldBet(CurrentHandContext&, bool deterministic = false) = 0;

    const std::string& getStrategyName() const { return myStrategyName; }
    void setStrategyName(const std::string& name) { myStrategyName = name; }
    const std::unique_ptr<PreflopRangeCalculator>& getPreflopRangeCalculator() const { return preflopRangeCalculator; }

    virtual ~IBotStrategy() = default;

  protected:
    void initializeRanges(const int utgHeadsUpRange, const int utgFullTableRange);

    int computePreflopRaiseAmount(CurrentHandContext&, bool deterministic = false);
    bool shouldPotControl(CurrentHandContext&, bool deterministic = false);

    bool myShouldCall = false;
    bool myShouldRaise = false;
    std::string myStrategyName;
    ILogger* myLogger;

  private:
    std::unique_ptr<PreflopRangeCalculator> preflopRangeCalculator;
};
} // namespace pkt::core::player
