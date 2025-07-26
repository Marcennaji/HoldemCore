#pragma once

namespace pkt::core
{
class IRandomizer
{
  public:
    virtual void getRand(int minValue, int maxValue, unsigned count, int* out) = 0;
    virtual ~IRandomizer() = default;
};
} // namespace pkt::core
