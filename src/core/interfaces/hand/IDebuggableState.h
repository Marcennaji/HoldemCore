#pragma once

namespace pkt::core
{

class HandFsm;

class IDebuggableState
{
  public:
    virtual ~IDebuggableState() = default;

    virtual void logStateInfo(const HandFsm& hand) const
    {
        // Optional default: do nothing
    }
};

} // namespace pkt::core
