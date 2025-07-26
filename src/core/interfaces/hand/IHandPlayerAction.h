#pragma once

#include "core/engine/model/PlayerAction.h"

namespace pkt::core
{
class IHandPlayerAction
{
  public:
    virtual ~IHandPlayerAction() = default;
    virtual void processPlayerAction(PlayerAction action) = 0;
};
} // namespace pkt::core
