
#pragma once

#include "EngineTest.h"

namespace pkt::test
{

class BettingRoundsLegacyTest : public EngineTest
{
  protected:
    void SetUp() override;
    void logTestMessage(const std::string& message) const;
    bool isPlayerStillActive(unsigned id) const;

  private:
    void dealBettingRoundCards(int bettingRoundId);
    void bettingRoundAnimation(int myBettingRoundID);
};
} // namespace pkt::test