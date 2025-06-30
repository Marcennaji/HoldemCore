
#pragma once

#include "EngineTest.h"

namespace pkt::test
{

class BettingRoundsLegacyTest : public EngineTest
{
  protected:
    void SetUp() override;

  private:
    void uiDealBettingRoundCards(int bettingRoundId);
};
} // namespace pkt::test