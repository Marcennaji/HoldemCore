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
class LooseAggressiveBotStrategy : public BotStrategyBase
{

  public:
    LooseAggressiveBotStrategy();
    
    // ISP-compliant constructor - only accepts what it actually needs (Logger + Randomizer)
    LooseAggressiveBotStrategy(std::shared_ptr<pkt::core::Logger> logger, 
                               std::shared_ptr<pkt::core::Randomizer> randomizer);
    
    ~LooseAggressiveBotStrategy();

  private:
    // ISP-compliant service dependencies - direct core interfaces (no wrapper complexity)
    std::shared_ptr<pkt::core::Logger> m_loggerService;
    std::shared_ptr<pkt::core::Randomizer> m_randomizerService;
    
    // Helper methods following Single Responsibility Principle  
    pkt::core::Logger& getLogger();
    pkt::core::Randomizer& getRandomizer();
    
    // Temporary adapter for legacy PokerMath calls - will be eliminated when PokerMath is refactored to ISP
    std::shared_ptr<pkt::core::ServiceContainer> createTemporaryServiceContainer();

  protected:
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
};

} // namespace pkt::core::player
