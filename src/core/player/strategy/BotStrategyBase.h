#pragma once

#include "core/engine/model/PlayerAction.h"
#include "core/player/strategy/BotStrategy.h"

namespace pkt::core::player
{
struct CurrentHandContext;

class BotStrategyBase : public virtual BotStrategy
{
  protected:
    pkt::core::PlayerAction decidePreflop(const CurrentHandContext&) override;
    pkt::core::PlayerAction decideFlop(const CurrentHandContext&) override;
    pkt::core::PlayerAction decideTurn(const CurrentHandContext&) override;
    pkt::core::PlayerAction decideRiver(const CurrentHandContext&) override;

    int computePreflopRaiseAmount(const CurrentHandContext&);
    bool shouldPotControl(const CurrentHandContext&);
    bool isPossibleToBluff(const CurrentHandContext& ctx) const;
    bool myShouldCall;
    bool myShouldRaise;

  private:
    virtual bool preflopShouldCall(const CurrentHandContext& ctx) = 0;
    virtual bool flopShouldCall(const CurrentHandContext& ctx) = 0;
    virtual bool turnShouldCall(const CurrentHandContext& ctx) = 0;
    virtual bool riverShouldCall(const CurrentHandContext& ctx) = 0;

    virtual int preflopShouldRaise(const CurrentHandContext& ctx) = 0;
    virtual int flopShouldRaise(const CurrentHandContext& ctx) = 0;
    virtual int turnShouldRaise(const CurrentHandContext& ctx) = 0;
    virtual int riverShouldRaise(const CurrentHandContext& ctx) = 0;

    virtual int flopShouldBet(const CurrentHandContext& ctx) = 0;
    virtual int turnShouldBet(const CurrentHandContext& ctx) = 0;
    virtual int riverShouldBet(const CurrentHandContext& ctx) = 0;

    int computeFirstRaiseAmount(const CurrentHandContext&, int bigBlind) const;
    void adjustRaiseForPosition(const CurrentHandContext&, int& raiseAmount, int bigBlind) const;
    void adjustRaiseForLimpers(const CurrentHandContext&, int& raiseAmount, int bigBlind) const;
    int computeReRaiseAmount(const CurrentHandContext&, int bigBlind) const;
    int computeThreeBetAmount(const CurrentHandContext&, int totalPot) const;
    int computeFourBetOrMoreAmount(const CurrentHandContext&, int totalPot) const;
    int finalizeRaiseAmount(const CurrentHandContext&, int raiseAmount) const;

    bool shouldPotControlForPocketPair(const CurrentHandContext&) const;
    bool shouldPotControlForFullHousePossibility(const CurrentHandContext&) const;
    bool shouldPotControlOnFlop(const CurrentHandContext&, int bigBlind) const;
    bool shouldPotControlOnTurn(const CurrentHandContext&, int bigBlind) const;

    void logPotControl() const;
};
} // namespace pkt::core::player
