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
class ManiacBotStrategy : public BotStrategyBase
{

  public:
    // Legacy default constructor for backward compatibility
    ManiacBotStrategy();
    
    // ISP-compliant constructor - only depends on what it needs
    ManiacBotStrategy(std::shared_ptr<pkt::core::Logger> logger, 
                     std::shared_ptr<pkt::core::Randomizer> randomizer);
    
    ~ManiacBotStrategy();

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

  private:
    // Helper methods to get services from either focused interfaces or legacy container
    pkt::core::Logger& getLogger();
    pkt::core::Randomizer& getRandomizer();
    
    // ISP-compliant focused dependencies
    std::shared_ptr<pkt::core::Logger> m_loggerService;
    std::shared_ptr<pkt::core::Randomizer> m_randomizerService;
};

} // namespace pkt::core::player
