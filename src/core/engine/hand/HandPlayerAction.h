#pragma once

#include "core/engine/model/PlayerAction.h"

namespace pkt::core
{
class HandPlayerAction
{
  public:
    virtual ~HandPlayerAction() = default;
    virtual void handlePlayerAction(PlayerAction action) = 0;
};
} // namespace pkt::core
