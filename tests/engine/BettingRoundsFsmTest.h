
#pragma once

#include "EngineTest.h"

namespace pkt::test
{

class BettingRoundsFsmTest : public EngineTest
{
  protected:
    void SetUp() override;
    void logTestMessage(const std::string& message) const;
    bool isPlayerStillActive(unsigned id) const;

  private:
    void dealBettingRoundCards(int bettingRoundId);
    void bettingRoundAnimation(int myBettingRoundId);
};
} // namespace pkt::test