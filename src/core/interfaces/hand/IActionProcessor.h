#pragma once

#include "core/engine/model/PlayerAction.h"

#include <memory>

namespace pkt::core
{
class IHand;
class IHandState;

class IActionProcessor
{
  public:
    virtual ~IActionProcessor() = default;

    virtual std::unique_ptr<IHandState> processAction(IHand& hand, PlayerAction action) = 0;
    virtual bool canProcessAction(const IHand& hand, PlayerAction action) const = 0;
};
} // namespace pkt::core
