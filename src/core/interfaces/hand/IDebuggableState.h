#pragma once

namespace pkt::core
{

class IHand;

class IDebuggableState
{
  public:
    virtual ~IDebuggableState() = default;

    virtual void logStateInfo(const IHand& hand) const
    {
        // Optional default: do nothing
    }
};

} // namespace pkt::core
