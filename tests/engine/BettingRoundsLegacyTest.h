
#pragma once

#include "EngineTest.h"

namespace pkt::test
{

class BettingRoundsLegacyTest : public EngineTest
{
  protected:
    void SetUp() override;

  private:
    // events that are part of the GameEvents interface
    void dealBettingRoundCards(int bettingRoundId);
    void activePlayerActionDone();
    void bettingRoundAnimation(int myBettingRoundID);
    void startPreflop();
    void startFlop();
    void startTurn();
    void startRiver();
    void startPostRiver();

    // internal gui methods that are called by the legacy qt widget ui, after the events are triggered
    void resolveHandConditions();

    // misc
    void logTestMessage(const std::string& message) const;
};
} // namespace pkt::test