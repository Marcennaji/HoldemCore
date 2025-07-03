
#pragma once

#include "EngineTest.h"

namespace pkt::test
{

class BettingRoundsLegacyTest : public EngineTest
{
  protected:
    void SetUp() override;

  private:
    void dealBettingRoundCards(int bettingRoundId);
    void bettingRoundAnimation(int myBettingRoundID);
    void logTestMessage(const std::string& message) const;
};
} // namespace pkt::test