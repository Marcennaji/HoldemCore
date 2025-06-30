// tests/BettingRoundsLegacyTest.cpp

#include "BettingRoundsLegacyTest.h"

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

void BettingRoundsLegacyTest::SetUp()
{
    EngineTest::SetUp();

    myEvents.onDealBettingRoundCards = [this](int bettingRoundId) { uiDealBettingRoundCards(bettingRoundId); };
}
void BettingRoundsLegacyTest::uiDealBettingRoundCards(int myBettingRoundID)
{

    switch (myBettingRoundID)
    {

    case 1:
    {
        // dealFlopCards0();
    }
    break;
    case 2:
    {
        // dealTurnCards0();
    }
    break;
    case 3:
    {
        // dealRiverCards0();
    }
    break;
    default:
    {
    }
    }
}

TEST_F(BettingRoundsLegacyTest, DISABLED_StartShouldSetPreflopAsCurrentRound)
{
    initializeHandForTesting(4);
    myHand->start();
    EXPECT_EQ(myHand->getCurrentRound(), GameStatePreflop);
}

TEST_F(BettingRoundsLegacyTest, DISABLED_SwitchRoundsGoesFromPreflopToFlop)
{
    initializeHandForTesting(4);
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
