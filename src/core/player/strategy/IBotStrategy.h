
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

class IBotPreflopStrategy
{
  public:
    virtual ~IBotPreflopStrategy() = default;

    virtual bool preflopShouldCall(CurrentHandContext&) = 0;
    virtual int preflopShouldRaise(CurrentHandContext&) = 0;

    int computePreflopRaiseAmount(CurrentHandContext&);

  private:
    int computeFirstRaiseAmount(const CurrentHandContext& ctx, int bigBlind) const;

    // Adjust raise amount based on player position
    void adjustRaiseForPosition(const CurrentHandContext& ctx, int& raiseAmount, int bigBlind) const;

    // Adjust raise amount based on limpers
    void adjustRaiseForLimpers(const CurrentHandContext& ctx, int& raiseAmount, int bigBlind) const;

    // Compute re-raise amount (3-bet, 4-bet, etc.)
    int computeReRaiseAmount(const CurrentHandContext& ctx, int bigBlind) const;

    // Compute 3-bet amount
    int computeThreeBetAmount(const CurrentHandContext& ctx, int totalPot) const;

    // Compute 4-bet or more amount
    int computeFourBetOrMoreAmount(const CurrentHandContext& ctx, int totalPot) const;

    // Finalize the raise amount (e.g., ensure it doesn't exceed player's cash)
    int finalizeRaiseAmount(const CurrentHandContext& ctx, int raiseAmount) const;
};

class IBotFlopStrategy
{
  public:
    virtual ~IBotFlopStrategy() = default;

    virtual bool flopShouldCall(CurrentHandContext&) = 0;
    virtual int flopShouldRaise(CurrentHandContext&) = 0;
    virtual int flopShouldBet(CurrentHandContext&) = 0;
};
class IBotTurnStrategy
{
  public:
    virtual ~IBotTurnStrategy() = default;

    virtual bool turnShouldCall(CurrentHandContext&) = 0;
    virtual int turnShouldRaise(CurrentHandContext&) = 0;
    virtual int turnShouldBet(CurrentHandContext&) = 0;
};

class IBotRiverStrategy
{
  public:
    virtual ~IBotRiverStrategy() = default;

    virtual bool riverShouldCall(CurrentHandContext&) = 0;
    virtual int riverShouldRaise(CurrentHandContext&) = 0;
    virtual int riverShouldBet(CurrentHandContext&) = 0;
};

class IBotStrategy : public IBotPreflopStrategy,
                     public IBotFlopStrategy,
                     public IBotTurnStrategy,
                     public IBotRiverStrategy
{
  public:
    const std::string& getStrategyName() const { return myStrategyName; }
    const std::unique_ptr<PreflopRangeCalculator>& getPreflopRangeCalculator() const
    {
        return myPreflopRangeCalculator;
    }

    virtual ~IBotStrategy() = default;

  protected:
    IBotStrategy(const std::string& name)
        : myStrategyName(name), myPreflopRangeCalculator(std::make_unique<PreflopRangeCalculator>())
    {
    }
    void initializeRanges(const int utgHeadsUpRange, const int utgFullTableRange);

    bool shouldPotControl(CurrentHandContext&);

    bool myShouldCall = false;
    bool myShouldRaise = false;
    const std::string myStrategyName;

  private:
    bool shouldPotControlForPocketPair(const CurrentHandContext& ctx) const;
    bool shouldPotControlForFullHousePossibility(const CurrentHandContext& ctx) const;
    bool shouldPotControlOnFlop(const CurrentHandContext& ctx, int bigBlind) const;
    bool shouldPotControlOnTurn(const CurrentHandContext& ctx, int bigBlind) const;
    void logPotControl() const;
    std::unique_ptr<PreflopRangeCalculator> myPreflopRangeCalculator;
};
} // namespace pkt::core::player
