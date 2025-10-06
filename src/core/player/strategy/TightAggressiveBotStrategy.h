// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#pragma once

#include "core/player/strategy/BotStrategy.h"
#include "core/player/strategy/BotStrategyBase.h"
#include "core/interfaces/Logger.h"
#include "core/interfaces/Randomizer.h"

namespace pkt::core::player
{
// Forward declaration
struct CurrentHandContext;
class TightAggressiveBotStrategy : public BotStrategyBase
{

  public:
    // Legacy default constructor for backward compatibility
    TightAggressiveBotStrategy();
    
    // ISP-compliant constructor using focused service interfaces
    TightAggressiveBotStrategy(std::shared_ptr<pkt::core::Logger> logger, std::shared_ptr<pkt::core::Randomizer> randomizer);
    ~TightAggressiveBotStrategy();

  private:
    virtual bool preflopCouldCall(const CurrentHandContext& ctx);
    virtual bool flopCouldCall(const CurrentHandContext& ctx);
    virtual bool turnCouldCall(const CurrentHandContext& ctx);
    virtual bool riverCouldCall(const CurrentHandContext& ctx);

    virtual int preflopCouldRaise(const CurrentHandContext& ctx);
    virtual int flopCouldRaise(const CurrentHandContext& ctx);
    virtual int turnCouldRaise(const CurrentHandContext& ctx);
    virtual int riverCouldRaise(const CurrentHandContext& ctx);

    virtual int flopCouldBet(const CurrentHandContext& ctx);
    virtual int turnCouldBet(const CurrentHandContext& ctx);
    virtual int riverCouldBet(const CurrentHandContext& ctx);

  protected:
    // ISP-compliant service access helpers
    pkt::core::Logger& getLogger() const;
    pkt::core::Randomizer& getRandomizer() const;

  private:
    // ISP-compliant focused service interfaces
    std::shared_ptr<pkt::core::Logger> m_logger;
    std::shared_ptr<pkt::core::Randomizer> m_randomizer;
};

} // namespace pkt::core::player
