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

    // Essential events for game flow
    myEvents.onActivePlayerActionDone = [this]() { activePlayerActionDone(); };
    myEvents.onBettingRoundAnimation = [this](int bettingRoundId) { bettingRoundAnimation(bettingRoundId); };
    myEvents.onDealBettingRoundCards = [this](int bettingRoundId) { dealBettingRoundCards(bettingRoundId); };
    myEvents.onStartPreflop = [this]() { startPreflop(); };
    myEvents.onStartFlop = [this]() { startFlop(); };
    myEvents.onStartTurn = [this]() { startTurn(); };
    myEvents.onStartRiver = [this]() { startRiver(); };
    myEvents.onStartPostRiver = [this]() { startPostRiver(); };
}
void BettingRoundsLegacyTest::resolveHandConditions()
{
    logTestMessage("myHand->resolveHandConditions() called");
    myHand->resolveHandConditions();
}

void BettingRoundsLegacyTest::activePlayerActionDone()
{
    logTestMessage("resolveHandConditions() called");
    resolveHandConditions();
}

void BettingRoundsLegacyTest::startPreflop()
{
    logTestMessage("Starting Preflop round");
    myHand->getCurrentBettingRound()->run();
}

void BettingRoundsLegacyTest::startFlop()
{
    logTestMessage("Starting Flop round");
    myHand->getCurrentBettingRound()->run();
}

void BettingRoundsLegacyTest::startTurn()
{
    logTestMessage("Starting Turn round");
    myHand->getCurrentBettingRound()->run();
}

void BettingRoundsLegacyTest::startRiver()
{
    logTestMessage("Starting River round");
    myHand->getCurrentBettingRound()->run();
}

void BettingRoundsLegacyTest::startPostRiver()
{
    logTestMessage("Starting Post-River round");
    myHand->getCurrentBettingRound()->postRiverRun();
}

void BettingRoundsLegacyTest::bettingRoundAnimation(int bettingRoundID)
{
    assert(myHand->getCurrentBettingRound()->getBettingRoundID() == bettingRoundID &&
           "Betting round ID does not match the current betting round ID");

    if (bettingRoundID < 4)
    {
        myHand->getCurrentBettingRound()->nextPlayer();
    }
}

void BettingRoundsLegacyTest::dealBettingRoundCards(int bettingRoundId)
{
    logTestMessage("Dealing cards for betting round: " + std::to_string(bettingRoundId));

    if (bettingRoundId != 0)
    {
        resolveHandConditions();
    }
}

// Tests for betting rounds and transitions

TEST_F(BettingRoundsLegacyTest, StartShouldGoFromPreflopToPostRiver)
{
    initializeHandForTesting(3);
    myHand->start();
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStatePostRiver);
}

} // namespace pkt::test
