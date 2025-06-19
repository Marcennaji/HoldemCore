
#pragma once
#include <memory>
#include "core/engine/Hand.h"

namespace pkt::core
{

class IBettingRoundState
{
  public:
    virtual ~IBettingRoundState() = default;
    virtual void enter(Hand& hand) = 0;
    virtual std::unique_ptr<IBettingRoundState> next() = 0;
};

} // namespace pkt::core
