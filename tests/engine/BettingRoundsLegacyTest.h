
#pragma once

#include "EngineTest.h"

namespace pkt::test
{

class BettingRoundsLegacyTest : public EngineTest
{
  protected:
    void SetUp() override;
    void TearDown() override;
    void logTestMessage(const std::string& message) const;
    bool isPlayerStillActive(unsigned id) const;
};
} // namespace pkt::test