#pragma once

#include "core/interfaces/IRandomizer.h"

#include <vector>

namespace pkt::test
{
class FakeRandomizer : public pkt::core::IRandomizer
{
  public:
    std::vector<int> values;
    size_t index = 0;

    void getRand(int minValue, int maxValue, unsigned count, int* out) override
    {
        for (unsigned i = 0; i < count; ++i)
        {
            if (index < values.size())
                *out++ = values[index++];
            else
                *out++ = minValue; // fallback
        }
    }
};

} // namespace pkt::test