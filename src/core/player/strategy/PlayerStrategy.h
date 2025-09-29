#pragma once

#include "core/engine/model/PlayerAction.h"

#include <string>

namespace pkt::core::player
{

struct CurrentHandContext;

class PlayerStrategy
{
  public:
    virtual ~PlayerStrategy() = default;

    virtual pkt::core::PlayerAction decideAction(const CurrentHandContext& ctx) = 0;
};

} // namespace pkt::core::player
