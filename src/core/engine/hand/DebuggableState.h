#pragma once

#include "core/engine/hand/Hand.h"
#include "core/engine/model/GameState.h"
#include "core/engine/game/Board.h"
#include "core/interfaces/Logger.h"

namespace pkt::core
{

class Hand;

class HandDebuggableState
{
  public:
    virtual ~HandDebuggableState() = default;

    virtual void logStateInfo(Hand& hand) = 0;
};

} // namespace pkt::core
