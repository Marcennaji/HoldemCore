#pragma once

#include "core/engine/model/PlayerAction.h"

#include <memory>

namespace pkt::core
{
class HandFsm;
class IHandState;

class IActionProcessor
{
  public:
    virtual ~IActionProcessor() = default;

    virtual std::unique_ptr<IHandState> processAction(HandFsm& hand, PlayerAction action) = 0;
    virtual bool canProcessAction(const HandFsm& hand, PlayerAction action) const = 0;
};
} // namespace pkt::core
