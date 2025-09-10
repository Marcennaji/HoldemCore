#pragma once

namespace pkt::core
{

class IHandLifecycle
{
  public:
    virtual ~IHandLifecycle() = default;
    virtual void initialize() = 0;
    virtual void runGameLoop() = 0;
    virtual void end() = 0;
};
} // namespace pkt::core
