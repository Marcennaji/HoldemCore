// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once

#include "core/engine/hand/Hand.h"
#include "core/engine/model/GameState.h"
#include "core/engine/game/Board.h"
#include "core/ports/Logger.h"

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
