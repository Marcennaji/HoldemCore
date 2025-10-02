#pragma once

#include "core/interfaces/Randomizer.h"

#include <vector>

namespace pkt::test
{
class FakeRandomizer : public pkt::core::Randomizer
{
  public:
    std::vector<int> values;
    size_t index = 0;

    void getRand(int minValue, int maxValue, unsigned count, int* out) override
    {
        for (unsigned i = 0; i < count; ++i)
        {
            if (index < values.size())
            {
                // Constrain the fake value to the requested range
                int rawValue = values[index++];
                int range = maxValue - minValue + 1;
                *out++ = minValue + (rawValue % range);
            }
            else
                *out++ = minValue; // fallback
        }
    }
};

} // namespace pkt::test