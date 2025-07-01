// tests/BettingRoundsLegacyTest.cpp

#include "BettingRoundsLegacyTest.h"
#include "core/services/GlobalServices.h"

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{
void BettingRoundsLegacyTest::logTestMessage(const std::string& message) const
{
    GlobalServices::instance().logger()->verbose("BettingRoundsLegacyTest : " + message, 2);
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
    logTestMessage("Calling resolveHandConditions(), current round before: " +
                   std::to_string(myHand->getCurrentRoundState()));

    assert(myHand->getRunningPlayerList()->size() == myHand->getActivePlayerList()->size() &&
           myHand->getRunningPlayerList()->size() == myPlayerList->size());

    myHand->resolveHandConditions();

    logTestMessage("After resolveHandConditions(), current round is: " +
                   std::to_string(myHand->getCurrentRoundState()));

    assert(myHand->getRunningPlayerList()->size() == myHand->getActivePlayerList()->size() &&
           myHand->getRunningPlayerList()->size() == myPlayerList->size());
}

void BettingRoundsLegacyTest::activePlayerActionDone()
{
    // After each player action, check if the betting round is complete
    // This is done by checking if all bets are done, and if so, proceed to next round
    // We do this by calling resolveHandConditions which handles special cases
    // and then checking if the round should progress
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
    initializeHandForTesting(6);
    myHand->start();
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStatePostRiver);
}

} // namespace pkt::test
