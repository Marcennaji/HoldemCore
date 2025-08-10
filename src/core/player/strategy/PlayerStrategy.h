#pragma once

#include "core/engine/model/PlayerAction.h"

#include <string>

namespace pkt::core::player
{

class CurrentHandContext;

class PlayerStrategy
{
  public:
    virtual ~PlayerStrategy() = default;

    // This is the only method FSM needs to call
    virtual PlayerAction decideAction(const CurrentHandContext& ctx) = 0;
};

} // namespace pkt::core::player
