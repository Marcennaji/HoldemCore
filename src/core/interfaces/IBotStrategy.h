
#pragma once

#include <core/player/Player.h>

namespace pkt::core
{

class IBotStrategy
{
  public:
    virtual bool preflopShouldCall(Player&) = 0;
    virtual bool flopShouldCall(Player&) = 0;
    virtual bool turnShouldCall(Player&) = 0;
    virtual bool riverShouldCall(Player&) = 0;

    virtual bool preflopShouldRaise(Player&) = 0;
    virtual bool flopShouldRaise(Player&) = 0;
    virtual bool turnShouldRaise(Player&) = 0;
    virtual bool riverShouldRaise(Player&) = 0;

    virtual bool flopShouldBet(Player&) = 0;
    virtual bool turnShouldBet(Player&) = 0;
    virtual bool riverShouldBet(Player&) = 0;

    virtual ~IBotStrategy() = default;
};
} // namespace pkt::core
