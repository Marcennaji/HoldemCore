// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once

#include "core/engine/model/PlayerAction.h"
#include "core/player/strategy/BotStrategy.h"
#include "core/ports/Logger.h"
#include "core/ports/Randomizer.h"

#include <memory>

namespace pkt::core::player
{
struct CurrentHandContext;

/**
 * @brief Base implementation class providing common functionality for bot strategies.
 * 
 * This class serves as the foundation for concrete bot strategy implementations,
 * providing shared decision-making logic, helper methods, and common patterns
 * used across different bot personalities and playing styles.
 */
class BotStrategyBase : public virtual BotStrategy
{
  public:
    BotStrategyBase(Logger& logger, Randomizer& randomizer);

  protected:
    pkt::core::PlayerAction decidePreflop(const CurrentHandContext&) override;
    pkt::core::PlayerAction decideFlop(const CurrentHandContext&) override;
    pkt::core::PlayerAction decideTurn(const CurrentHandContext&) override;
    pkt::core::PlayerAction decideRiver(const CurrentHandContext&) override;

    int computePreflopRaiseAmount(const CurrentHandContext&);
    bool shouldPotControl(const CurrentHandContext&);
    bool isPossibleToBluff(const CurrentHandContext& ctx) const;
    bool m_couldCall;
    bool m_couldRaise;

    Logger& m_logger;
    mutable Randomizer* m_randomizer; 

  private:
    virtual bool preflopCouldCall(const CurrentHandContext& ctx) = 0;
    virtual bool flopCouldCall(const CurrentHandContext& ctx) = 0;
    virtual bool turnCouldCall(const CurrentHandContext& ctx) = 0;
    virtual bool riverCouldCall(const CurrentHandContext& ctx) = 0;

    virtual int preflopCouldRaise(const CurrentHandContext& ctx) = 0;
    virtual int flopCouldRaise(const CurrentHandContext& ctx) = 0;
    virtual int turnCouldRaise(const CurrentHandContext& ctx) = 0;
    virtual int riverCouldRaise(const CurrentHandContext& ctx) = 0;

    virtual int flopCouldBet(const CurrentHandContext& ctx) = 0;
    virtual int turnCouldBet(const CurrentHandContext& ctx) = 0;
    virtual int riverCouldBet(const CurrentHandContext& ctx) = 0;

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

    bool canAffordToCall(const CurrentHandContext& ctx) const;
    bool canAffordToRaise(const CurrentHandContext& ctx, int raiseAmount) const;
    int getCallAmount(const CurrentHandContext& ctx) const;
};
} // namespace pkt::core::player
