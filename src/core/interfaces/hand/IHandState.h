#pragma once

#include "core/engine/model/GameState.h"

#include <string>

namespace pkt::core
{

class HandFsm;

class IHandState
{
  public:
    virtual ~IHandState() = default;

    virtual void enter(HandFsm& hand) = 0;
    virtual void exit(HandFsm& hand) = 0;
    virtual bool isTerminal() const { return false; }

    virtual const GameState getGameState() const = 0;
};
} // namespace pkt::core
