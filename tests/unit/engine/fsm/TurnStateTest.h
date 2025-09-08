#pragma once

#include "common/EngineTest.h"

namespace pkt::test
{

class TurnStateTest : public EngineTest
{
  protected:
    void SetUp() override;
    void TearDown() override;
    void logTestMessage(const std::string& message) const;
};
} // namespace pkt::test
