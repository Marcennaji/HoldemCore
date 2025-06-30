// tests/BettingRoundsLegacyTest.cpp

#include "BettingRoundsLegacyTest.h"

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

void BettingRoundsLegacyTest::SetUp()
{
    EngineTest::SetUp();

    myEvents.onStartPreflop = [this]() { startPreflop(); };
    myEvents.onBettingRoundAnimation = [this](int bettingRoundId) { bettingRoundAnimation(bettingRoundId); };
    myEvents.onActivePlayerActionDone = [this]()
    {
        // This is where we would handle the end of a player's action in the betting round
        // For now, we just switch rounds to simulate the next player taking their turn
        handSwitchRounds();
    };
}
void BettingRoundsLegacyTest::handSwitchRounds()
{
    myHand->switchRounds();
}

void BettingRoundsLegacyTest::startPreflop()
{
    myHand->getCurrentBettingRound()->run();
}

void BettingRoundsLegacyTest::bettingRoundAnimation(int myBettingRoundID)
{

    switch (myBettingRoundID)
    {

    case 0:
    {
        preflopAnimation2();
    }
    break;
    case 1:
    {
        // flopAnimation2();
    }
    break;
    case 2:
    {
        // turnAnimation2();
    }
    break;
    case 3:
    {
        // riverAnimation2();
    }
    break;
    default:
    {
    }
    }
}
void BettingRoundsLegacyTest::preflopAnimation2()
{
    myHand->getCurrentBettingRound()->nextPlayer();
}

// Tests for betting rounds and transitions

TEST_F(BettingRoundsLegacyTest, DISABLED_StartShouldSetPreflopAsCurrentRound)
{
    initializeHandForTesting(4);
    myHand->start();
    EXPECT_EQ(myHand->getCurrentRound(), GameStatePreflop);
}

TEST_F(BettingRoundsLegacyTest, DISABLED_SwitchRoundsGoesFromPreflopToFlop)
{
    initializeHandForTesting(2);
    myHand->start();
    myHand->switchRounds();
    EXPECT_EQ(myHand->getCurrentRound(), GameStateFlop);
}

TEST_F(BettingRoundsLegacyTest, DISABLED_SwitchRoundsGoesThroughAllRoundsToRiver)
{
    initializeHandForTesting(4);
    myHand->start();
    myHand->switchRounds(); // Preflop -> Flop
    EXPECT_EQ(myHand->getCurrentRound(), GameStateFlop);
    myHand->switchRounds(); // Flop -> Turn
    EXPECT_EQ(myHand->getCurrentRound(), GameStateTurn);
    myHand->switchRounds(); // Turn -> River
    EXPECT_EQ(myHand->getCurrentRound(), GameStateRiver);
}

TEST_F(BettingRoundsLegacyTest, DISABLED_ShouldNotCrashIfSwitchRoundsCalledAfterRiver)
{
    initializeHandForTesting(4);
    myHand->start();
    myHand->switchRounds(); // Flop
    myHand->switchRounds(); // Turn
    myHand->switchRounds(); // River
    EXPECT_EQ(myHand->getCurrentRound(), GameStateRiver);
    myHand->switchRounds();                               // Should be no-op or handled internally
    EXPECT_EQ(myHand->getCurrentRound(), GameStateRiver); // still river or finished
}

TEST_F(BettingRoundsLegacyTest, DISABLED_SwitchRounds_TransitionsCorrectlyThroughAllPhases)
{
    initializeHandForTesting(6);
    myHand->setCurrentRound(GameStatePreflop);
    myHand->setAllInCondition(false); // normal game

    myHand->switchRounds();
    EXPECT_EQ(myHand->getCurrentRound(), GameStateFlop);

    myHand->switchRounds();
    EXPECT_EQ(myHand->getCurrentRound(), GameStateTurn);

    myHand->switchRounds();
    EXPECT_EQ(myHand->getCurrentRound(), GameStateRiver);

    // After river, no more transitions
    myHand->switchRounds();
    EXPECT_EQ(myHand->getCurrentRound(), GameStateRiver);
}

TEST_F(BettingRoundsLegacyTest, DISABLED_SwitchRounds_DoesNotAdvanceWhenAllInConditionIsTrue)
{
    initializeHandForTesting(6);
    myHand->setCurrentRound(GameStateFlop);
    myHand->setAllInCondition(true); // simulate all-in stop

    myHand->switchRounds();
    EXPECT_EQ(myHand->getCurrentRound(), GameStateFlop);
}

TEST_F(BettingRoundsLegacyTest, DISABLED_SwitchRounds_DoesNotAdvancePastRiver)
{
    initializeHandForTesting(6);
    myHand->setCurrentRound(GameStateRiver);

    myHand->switchRounds(); // should not move past river
    EXPECT_EQ(myHand->getCurrentRound(), GameStateRiver);
}

TEST_F(BettingRoundsLegacyTest, DISABLED_SwitchRounds_SkipsWhenOnlyOnePlayerRunning)
{
    initializeHandForTesting(1);
    myHand->setCurrentRound(GameStatePreflop);

    myHand->switchRounds();                                 // should detect end of game
    EXPECT_EQ(myHand->getCurrentRound(), GameStatePreflop); // or GameStateRiver if end state forced
}

} // namespace pkt::test
