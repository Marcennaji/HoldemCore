#pragma once

#include <gtest/gtest.h>
#include "common/EngineTest.h"

namespace pkt::test
{

class PostRiverStateTest : public EngineTest
{
  protected:
    void SetUp() override;
    void TearDown() override;

    void logTestMessage(const std::string& message);
};

} // namespace pkt::test
