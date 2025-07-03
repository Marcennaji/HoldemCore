// tests/BettingRoundsLegacyTest.cpp

#include "BettingRoundsLegacyTest.h"
#include "core/services/GlobalServices.h"

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{
void BettingRoundsLegacyTest::logTestMessage(const std::string& message) const
{
    GlobalServices::instance().logger()->verbose("BettingRoundsLegacyTest : " + message);
}

void BettingRoundsLegacyTest::SetUp()
{
    EngineTest::SetUp();

    // Essential events for game flow, in the legacy code (that has an unwanted dependency in which the gui pilots the
    // game)
    myEvents.onActivePlayerActionDone = [this]() { myHand->resolveHandConditions(); };
    myEvents.onBettingRoundAnimation = [this](int bettingRoundId) { bettingRoundAnimation(bettingRoundId); };
    myEvents.onDealBettingRoundCards = [this](int bettingRoundId) { dealBettingRoundCards(bettingRoundId); };
    myEvents.onStartPreflop = [this]() { myHand->getCurrentBettingRound()->run(); };
    myEvents.onStartFlop = [this]() { myHand->getCurrentBettingRound()->run(); };
    myEvents.onStartTurn = [this]() { myHand->getCurrentBettingRound()->run(); };
    myEvents.onStartRiver = [this]() { myHand->getCurrentBettingRound()->run(); };
    myEvents.onStartPostRiver = [this]() { myHand->getCurrentBettingRound()->postRiverRun(); };
}

void BettingRoundsLegacyTest::bettingRoundAnimation(int bettingRoundID)
{
    if (bettingRoundID < 4)
    {
        myHand->getCurrentBettingRound()->nextPlayer();
    }
}

void BettingRoundsLegacyTest::dealBettingRoundCards(int bettingRoundId)
{
    if (bettingRoundId != 0)
    {
        myHand->resolveHandConditions();
    }
}

// Tests for betting rounds and transitions

TEST_F(BettingRoundsLegacyTest, StartShouldGoFromPreflopToPostRiverHeadsUp)
{
    initializeHandForTesting(2);
    myHand->start();
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStatePostRiver);
}
TEST_F(BettingRoundsLegacyTest, StartShouldGoFromPreflopToPostRiver3Players)
{
    initializeHandForTesting(3);
    myHand->start();
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStatePostRiver);
}
TEST_F(BettingRoundsLegacyTest, StartShouldGoFromPreflopToPostRiver6Players)
{
    initializeHandForTesting(6);
    myHand->start();
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStatePostRiver);
}

} // namespace pkt::test
