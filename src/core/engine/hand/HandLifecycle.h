#pragma once

namespace pkt::core
{

class HandLifecycle
{
  public:
    virtual ~HandLifecycle() = default;
    virtual void initialize() = 0;
    virtual void runGameLoop() = 0;
    virtual void end() = 0;
};
} // namespace pkt::core
