#pragma once

#include "common/EngineTest.h"

namespace pkt::test
{

class RiverStateTest : public EngineTest
{
  public:
    RiverStateTest() = default;
    ~RiverStateTest() override = default;

  protected:
    void SetUp() override;
    void TearDown() override;
    void logTestMessage(const std::string& message) const;
};

} // namespace pkt::test
