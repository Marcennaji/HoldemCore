
#pragma once

#include "EngineTest.h"

namespace pkt::test
{

class BettingRoundsLegacyTest : public EngineTest
{
  protected:
    void SetUp() override;

  private:
    void handSwitchRounds();
    void startPreflop();
    void bettingRoundAnimation(int myBettingRoundID);
    void preflopAnimation2();
};
} // namespace pkt::test