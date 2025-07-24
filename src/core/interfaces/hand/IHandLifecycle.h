#pragma once

namespace pkt::core
{

class IHandLifecycle
{
  public:
    virtual ~IHandLifecycle() = default;
    virtual void startHand() = 0;
    virtual void endHand() = 0;
};
} // namespace pkt::core
