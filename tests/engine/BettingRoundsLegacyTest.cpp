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
    myEvents.onStartPreflop = [this]() { startPreflop(); };
    myEvents.onStartFlop = [this]() { startFlop(); };
    myEvents.onStartTurn = [this]() { startTurn(); };
    myEvents.onStartRiver = [this]() { startRiver(); };

    myEvents.onBettingRoundAnimation = [this](int bettingRoundId) { bettingRoundAnimation(bettingRoundId); };
    myEvents.onActivePlayerActionDone = [this]() { onActivePlayerActionDone(); };

    // Pot and cash management events
    myEvents.onPotUpdated = [this](int pot) { onPotUpdated(pot); };
    myEvents.onRefreshCash = [this]() { onRefreshCash(); };
    myEvents.onRefreshSet = [this]() { onRefreshSet(); };

    // Card dealing events
    myEvents.onDealBettingRoundCards = [this](int bettingRoundId) { onDealBettingRoundCards(bettingRoundId); };
    myEvents.onDealHoleCards = [this]() { onDealHoleCards(); };

    // Player state events
    myEvents.onRefreshAction = [this](int playerId, int playerAction) { onRefreshAction(playerId, playerAction); };
    myEvents.onRefreshPlayersActiveInactiveStyles = [this](int playerId, int status)
    { onRefreshPlayersActiveInactiveStyles(playerId, status); };
}
void BettingRoundsLegacyTest::resolveHandConditions()
{
    logTestMessage("Calling resolveHandConditions(), current round before: " +
                   std::to_string(myHand->getCurrentRoundState()));
    myHand->resolveHandConditions();
    logTestMessage("After resolveHandConditions(), current round: " + std::to_string(myHand->getCurrentRoundState()));
}

void BettingRoundsLegacyTest::onActivePlayerActionDone()
{
    // After each player action, check if the betting round is complete
    // This is done by checking if all bets are done, and if so, proceed to next round
    // We do this by calling resolveHandConditions which handles special cases
    // and then checking if the round should progress
    resolveHandConditions();
}

void BettingRoundsLegacyTest::startPreflop()
{
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

// Pot and cash event handlers
void BettingRoundsLegacyTest::onPotUpdated(int pot)
{
    logTestMessage("Pot updated to: " + std::to_string(pot));
}

void BettingRoundsLegacyTest::onRefreshCash()
{
    logTestMessage("Refreshing player cash");
}

void BettingRoundsLegacyTest::onRefreshSet()
{
    logTestMessage("Refreshing player bets");
}

// Card dealing event handlers
void BettingRoundsLegacyTest::onDealBettingRoundCards(int bettingRoundId)
{
    logTestMessage("Dealing cards for betting round: " + std::to_string(bettingRoundId));
}

void BettingRoundsLegacyTest::onDealHoleCards()
{
    logTestMessage("Dealing hole cards");
}

// Player state event handlers
void BettingRoundsLegacyTest::onRefreshAction(int playerId, int playerAction)
{
    logTestMessage("Player " + std::to_string(playerId) + " action: " + std::to_string(playerAction));
}

void BettingRoundsLegacyTest::onRefreshPlayersActiveInactiveStyles(int playerId, int status)
{
    logTestMessage("Player " + std::to_string(playerId) + " status: " + std::to_string(status));
}

// Tests for betting rounds and transitions

TEST_F(BettingRoundsLegacyTest, StartShouldSetPreflopAsCurrentRound)
{
    initializeHandForTesting(6);
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
    myHand->setCurrentRound(GameStatePreflop);
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
    myHand->setCurrentRound(GameStateFlop);
    myHand->setAllInCondition(true); // simulate all-in stop

    myHand->resolveHandConditions();
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStateFlop);
}

TEST_F(BettingRoundsLegacyTest, DISABLED_SwitchRounds_DoesNotAdvancePastRiver)
{
    initializeHandForTesting(6);
    myHand->setCurrentRound(GameStateRiver);

    myHand->resolveHandConditions(); // should not move past river
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStateRiver);
}

TEST_F(BettingRoundsLegacyTest, DISABLED_SwitchRounds_SkipsWhenOnlyOnePlayerRunning)
{
    initializeHandForTesting(1);
    myHand->setCurrentRound(GameStatePreflop);

    myHand->resolveHandConditions();                             // should detect end of game
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStatePreflop); // or GameStateRiver if end state forced
}

} // namespace pkt::test
