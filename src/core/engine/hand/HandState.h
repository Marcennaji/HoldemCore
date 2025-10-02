#pragma once

#include "core/engine/model/GameState.h"

#include <string>

namespace pkt::core
{

class Hand;

class HandState
{
  public:
    virtual ~HandState() = default;

    virtual void enter(Hand& hand) = 0;
    virtual void exit(Hand& hand) = 0;
    virtual bool isTerminal() const { return false; }

    virtual const GameState getGameState() const = 0;
};
} // namespace pkt::core
