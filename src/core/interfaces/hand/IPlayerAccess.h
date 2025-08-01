#pragma once

#include "typedefs.h"

namespace pkt::core
{

class IPlayerAccess
{
  public:
    virtual ~IPlayerAccess() = default;
    virtual const pkt::core::player::PlayerFsmList getSeatsList() const = 0;
    virtual const pkt::core::player::PlayerFsmList getRunningPlayersList() const = 0;
};

} // namespace pkt::core