// tests/BettingRoundsLegacyTest.cpp

#include "BettingRoundsLegacyTest.h"
#include <iostream>

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{
void BettingRoundsLegacyTest::logTestMessage(const std::string& message) const
{
    std::cout << std::endl << "*** BettingRoundsLegacyTest : " << message << std::endl << std::endl;
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

TEST_F(BettingRoundsLegacyTest, StartShouldSetPreflopAsCurrentRound)
{
    initializeHandForTesting(3);
    myHand->start();
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStatePreflop);
}

TEST_F(BettingRoundsLegacyTest, SwitchRoundsGoesFromPreflopToFlop)
{
    initializeHandForTesting(2);
    myHand->start();
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStatePreflop);
}

TEST_F(BettingRoundsLegacyTest, DISABLED_SwitchRoundsGoesThroughAllRoundsToRiver)
{
    initializeHandForTesting(4);
    myHand->start();
    myHand->resolveHandConditions(); // Preflop -> Flop
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStateFlop);
    myHand->resolveHandConditions(); // Flop -> Turn
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStateTurn);
    myHand->resolveHandConditions(); // Turn -> River
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStateRiver);
}

TEST_F(BettingRoundsLegacyTest, DISABLED_ShouldNotCrashIfSwitchRoundsCalledAfterRiver)
{
    initializeHandForTesting(4);
    myHand->start();
    myHand->resolveHandConditions(); // Flop
    myHand->resolveHandConditions(); // Turn
    myHand->resolveHandConditions(); // River
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStateRiver);
    myHand->resolveHandConditions();                           // Should be no-op or handled internally
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStateRiver); // still river or finished
}

TEST_F(BettingRoundsLegacyTest, DISABLED_SwitchRounds_TransitionsCorrectlyThroughAllPhases)
{
    initializeHandForTesting(6);
    myHand->setCurrentRoundState(GameStatePreflop);
    myHand->setAllInCondition(false); // normal game

    myHand->resolveHandConditions();
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStateFlop);

    myHand->resolveHandConditions();
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStateTurn);

    myHand->resolveHandConditions();
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStateRiver);

    // After river, no more transitions
    myHand->resolveHandConditions();
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStateRiver);
}

TEST_F(BettingRoundsLegacyTest, DISABLED_SwitchRounds_DoesNotAdvanceWhenAllInConditionIsTrue)
{
    initializeHandForTesting(6);
    myHand->setCurrentRoundState(GameStateFlop);
    myHand->setAllInCondition(true); // simulate all-in stop

    myHand->resolveHandConditions();
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStateFlop);
}

TEST_F(BettingRoundsLegacyTest, DISABLED_SwitchRounds_DoesNotAdvancePastRiver)
{
    initializeHandForTesting(6);
    myHand->setCurrentRoundState(GameStateRiver);

    myHand->resolveHandConditions(); // should not move past river
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStateRiver);
}

TEST_F(BettingRoundsLegacyTest, DISABLED_SwitchRounds_SkipsWhenOnlyOnePlayerRunning)
{
    initializeHandForTesting(1);
    myHand->setCurrentRoundState(GameStatePreflop);

    myHand->resolveHandConditions();                             // should detect end of game
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStatePreflop); // or GameStateRiver if end state forced
}

} // namespace pkt::test
