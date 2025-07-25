#pragma once

#include "typedefs.h"

namespace pkt::core
{

class IPlayerAccess
{
  public:
    virtual ~IPlayerAccess() = default;
    virtual pkt::core::player::PlayerList getSeatsList() const = 0;
    virtual pkt::core::player::PlayerList getRunningPlayersList() const = 0;
    virtual pkt::core::player::PlayerListIterator getPlayerSeatFromId(unsigned) const = 0;
    virtual pkt::core::player::PlayerListIterator getRunningPlayerFromId(unsigned) const = 0;
};

} // namespace pkt::core