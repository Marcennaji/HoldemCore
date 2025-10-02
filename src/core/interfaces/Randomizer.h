#pragma once

namespace pkt::core
{
class Randomizer
{
  public:
    virtual void getRand(int minValue, int maxValue, unsigned count, int* out) = 0;
    virtual ~Randomizer() = default;
};
} // namespace pkt::core
