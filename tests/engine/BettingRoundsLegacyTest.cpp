// tests/BettingRoundsLegacyTest.cpp

#include "BettingRoundsLegacyTest.h"
#include <iostream>

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

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
    std::cout << "Calling resolveHandConditions(), current round before: " << myHand->getCurrentRoundState()
              << std::endl;
    myHand->resolveHandConditions();
    std::cout << "After resolveHandConditions(), current round: " << myHand->getCurrentRoundState() << std::endl;
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
    std::cout << "Starting Flop round" << std::endl;
    myHand->getCurrentBettingRound()->run();
}

void BettingRoundsLegacyTest::startTurn()
{
    std::cout << "Starting Turn round" << std::endl;
    myHand->getCurrentBettingRound()->run();
}

void BettingRoundsLegacyTest::startRiver()
{
    std::cout << "Starting River round" << std::endl;
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

void BettingRoundsLegacyTest::simulateBettingRoundToCompletion()
{
    // Store the current round to detect when it changes
    GameState initialRound = myHand->getCurrentRoundState();
    std::cout << "Starting simulation from round: " << initialRound << std::endl;

    int maxIterations = 10;
    int iteration = 0;

    // Keep calling run until the round changes or we hit max iterations
    while (myHand->getCurrentRoundState() == initialRound && iteration < maxIterations)
    {
        std::cout << "Iteration " << iteration << ", round: " << myHand->getCurrentRoundState()
                  << ", running players: " << myHand->getRunningPlayerList()->size()
                  << ", first round: " << myHand->getCurrentBettingRound()->getFirstRound() << std::endl;

        // Call run to process the betting round
        myHand->getCurrentBettingRound()->run();

        // After run, check hand conditions
        resolveHandConditions();

        iteration++;
    }

    std::cout << "Simulation completed after " << iteration
              << " iterations, final round: " << myHand->getCurrentRoundState() << std::endl;

    if (iteration >= maxIterations)
    {
        FAIL() << "Betting round simulation did not complete within " << maxIterations << " iterations";
    }
}

// Pot and cash event handlers
void BettingRoundsLegacyTest::onPotUpdated(int pot)
{
    std::cout << "Pot updated to: " << pot << std::endl;
}

void BettingRoundsLegacyTest::onRefreshCash()
{
    std::cout << "Refreshing player cash" << std::endl;
}

void BettingRoundsLegacyTest::onRefreshSet()
{
    std::cout << "Refreshing player bets" << std::endl;
}

// Card dealing event handlers
void BettingRoundsLegacyTest::onDealBettingRoundCards(int bettingRoundId)
{
    std::cout << "Dealing cards for betting round: " << bettingRoundId << std::endl;
}

void BettingRoundsLegacyTest::onDealHoleCards()
{
    std::cout << "Dealing hole cards" << std::endl;
}

// Player state event handlers
void BettingRoundsLegacyTest::onRefreshAction(int playerId, int playerAction)
{
    std::cout << "Player " << playerId << " action: " << playerAction << std::endl;
}

void BettingRoundsLegacyTest::onRefreshPlayersActiveInactiveStyles(int playerId, int status)
{
    std::cout << "Player " << playerId << " status: " << status << std::endl;
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

    // Simulate the betting round until it completes
    simulateBettingRoundToCompletion();
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStateFlop);
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
