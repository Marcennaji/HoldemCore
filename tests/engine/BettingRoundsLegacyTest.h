
#pragma once

#include "EngineTest.h"

namespace pkt::test
{

class BettingRoundsLegacyTest : public EngineTest
{
  protected:
    void SetUp() override;
    void simulateBettingRoundToCompletion();

  private:
    void resolveHandConditions();
    void onActivePlayerActionDone();

    // Round start events
    void startPreflop();
    void startFlop();
    void startTurn();
    void startRiver();

    // Animation events
    void bettingRoundAnimation(int myBettingRoundID);
    void preflopAnimation2();

    // Pot and cash events
    void onPotUpdated(int pot);
    void onRefreshCash();
    void onRefreshSet();

    // Card dealing events
    void onDealBettingRoundCards(int bettingRoundId);
    void onDealHoleCards();

    // Player state events
    void onRefreshAction(int playerId, int playerAction);
    void onRefreshPlayersActiveInactiveStyles(int playerId, int status);
};
} // namespace pkt::test