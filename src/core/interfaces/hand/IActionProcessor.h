#pragma once

#include "core/engine/model/PlayerAction.h"
#include "core/player/PlayerFsm.h"

#include <memory>

namespace pkt::core
{
class HandFsm;
class IHandState;

class IActionProcessor
{
  public:
    virtual ~IActionProcessor() = default;

    virtual void promptPlayerAction(HandFsm& hand, player::PlayerFsm& player) = 0;
    virtual std::unique_ptr<IHandState> computeNextState(HandFsm& hand, PlayerAction action) = 0;
    virtual bool isActionAllowed(const HandFsm& hand, PlayerAction action) const = 0;
};
} // namespace pkt::core
