#pragma once

namespace pkt::core
{

class IHandLifecycle
{
  public:
    virtual ~IHandLifecycle() = default;
    virtual void start() = 0;
    virtual void end() = 0;
};
} // namespace pkt::core
