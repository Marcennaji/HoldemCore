// tests/e2e/EventDrivenArchitectureTest.cpp

#include "EventDrivenArchitectureTest.h"
#include "common/DeterministicStrategy.h"
#include "core/engine/HandFsm.h"
#include "core/player/Helpers.h"
#include "core/player/PlayerFsm.h"
#include "core/services/GlobalServices.h"

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

void MockUI::clear()
{
    bettingRoundsStarted.clear();
    playerActions.clear();
    playerTurns.clear();
    gameStates.clear();
    chipUpdates.clear();
    potUpdates.clear();
    invalidActions.clear();
    engineErrors.clear();
    awaitingHumanInput = false;
    gameInitialized = false;
    gameSpeed = 0;
}

bool MockUI::canDetermineCurrentPlayer() const
{
    // Can the UI determine who should act next based on events?
    return !playerActions.empty() || !bettingRoundsStarted.empty();
}

bool MockUI::hasValidActionsForCurrentPlayer() const
{
    // Does the UI know what actions are valid for current player?
    // This is currently missing from events!
    return false; // TODO: This should be true when proper events exist
}

bool MockUI::canDisplayGameState() const
{
    // Can the UI display current game state without calling engine?
    return !gameStates.empty() || gameInitialized;
}

unsigned MockUI::getCurrentPlayerFromEvents() const
{
    // Try to determine current player from events alone
    if (!playerActions.empty())
    {
        // Would need next player logic, but that requires engine knowledge
        // This shows the architectural problem!
        return playerActions.back().playerId;
    }
    return 0;
}

void EventDrivenArchitectureTest::SetUp()
{
    EngineTest::SetUp();
    mockUI = std::make_unique<MockUI>();
    createGameEvents();
}
void EventDrivenArchitectureTest::createGameEvents()
{
    // Clear existing events and set up our mock UI events
    // This follows the pattern from BettingRoundsFsmTest
    myEvents.clear();

    myEvents.onGameInitialized = [this](int speed)
    {
        mockUI->gameInitialized = true;
        mockUI->gameSpeed = speed;
    };

    myEvents.onBettingRoundStarted = [this](GameState state) { mockUI->bettingRoundsStarted.push_back(state); };

    myEvents.onPlayerActed = [this](PlayerAction action) { mockUI->playerActions.push_back(action); };

    myEvents.onPotUpdated = [this](int newPot) { mockUI->potUpdates.push_back(newPot); };

    myEvents.onPlayerChipsUpdated = [this](unsigned playerId, int newChips)
    { mockUI->chipUpdates.push_back({playerId, newChips}); };

    myEvents.onInvalidPlayerAction = [this](unsigned playerId, PlayerAction invalidAction, std::string reason)
    { mockUI->invalidActions.push_back({playerId, invalidAction, reason}); };

    myEvents.onEngineError = [this](std::string errorMessage) { mockUI->engineErrors.push_back(errorMessage); };

    myEvents.onAwaitingHumanInput = [this]() { mockUI->awaitingHumanInput = true; };
}

/**
 * Test that engine fires events when betting rounds start
 * UI should know game state transitions without calling engine
 */
TEST_F(EventDrivenArchitectureTest, EngineFiresBettingRoundEvents)
{
    initializeHandFsmWithPlayers(2, gameData);

    // Configure simple strategies that will fold immediately to test events
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    // Run the hand
    myHandFsm->runGameLoop();

    // UI should have received betting round started event
    ASSERT_FALSE(mockUI->bettingRoundsStarted.empty())
        << "UI should receive betting round events without calling engine";

    EXPECT_EQ(static_cast<int>(mockUI->bettingRoundsStarted[0]), static_cast<int>(GameState::Preflop))
        << "First betting round should be Preflop. Got: " << static_cast<int>(mockUI->bettingRoundsStarted[0]);

    // Should also see PostRiver at the end
    bool foundPostRiver = false;
    for (auto state : mockUI->bettingRoundsStarted)
    {
        if (state == GameState::PostRiver)
        {
            foundPostRiver = true;
            break;
        }
    }
    EXPECT_TRUE(foundPostRiver) << "Should eventually see PostRiver state";
}

/**
 * Test that engine fires events when players act
 * UI should track all actions without polling engine
 */
TEST_F(EventDrivenArchitectureTest, EngineFiresPlayerActionEvents)
{
    initializeHandFsmWithPlayers(2, gameData);

    // Configure strategies for a complete hand
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Call});
    sbStrategy->setLastAction(GameState::Flop, {playerSb->getId(), ActionType::Check});
    sbStrategy->setLastAction(GameState::Turn, {playerSb->getId(), ActionType::Check});
    sbStrategy->setLastAction(GameState::River, {playerSb->getId(), ActionType::Check});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    bbStrategy->setLastAction(GameState::Preflop, {playerBb->getId(), ActionType::Check});
    bbStrategy->setLastAction(GameState::Flop, {playerBb->getId(), ActionType::Check});
    bbStrategy->setLastAction(GameState::Turn, {playerBb->getId(), ActionType::Check});
    bbStrategy->setLastAction(GameState::River, {playerBb->getId(), ActionType::Check});
    playerBb->setStrategy(std::move(bbStrategy));

    // Run the hand
    myHandFsm->runGameLoop();

    // UI should have received player actions
    EXPECT_GE(mockUI->playerActions.size(), 4u) << "UI should receive events for all player actions";

    // Check that actions include blind postings and regular actions
    bool hasBlindAction = false;
    bool hasRegularAction = false;

    for (const auto& action : mockUI->playerActions)
    {
        if (action.type == ActionType::PostSmallBlind || action.type == ActionType::PostBigBlind)
        {
            hasBlindAction = true;
        }
        else if (action.type == ActionType::Call || action.type == ActionType::Check)
        {
            hasRegularAction = true;
        }
    }

    EXPECT_TRUE(hasBlindAction) << "UI should receive blind posting events";
    EXPECT_TRUE(hasRegularAction) << "UI should receive regular action events";
}

/**
 * Test current architectural limitation: UI cannot determine valid actions
 * This test documents what's missing for proper hexagonal architecture
 */
TEST_F(EventDrivenArchitectureTest, DISABLED_UICannotDetermineValidActions)
{
    // Clear events before initializing the game
    mockUI->clear();

    // This test is disabled because it highlights the current limitation
    // In proper hexagonal architecture, the UI should know valid actions
    // for the current player through events, not by calling the engine directly

    initializeHandFsmWithPlayers(2, gameData);

    // What's missing for true hexagonal architecture:
    // 1. onPlayerTurnStarted(playerId, validActions, gameContext) event
    // 2. onValidActionsChanged(playerId, newValidActions) event
    // 3. onGameStateChanged(currentState, playerStates) event

    // Currently UI cannot determine valid actions without engine calls
    EXPECT_FALSE(mockUI->hasValidActionsForCurrentPlayer())
        << "This fails because current events don't provide valid actions";
}

/**
 * Test that UI can track pot changes through events
 */
TEST_F(EventDrivenArchitectureTest, UIReceivesPotUpdates)
{
    initializeHandFsmWithPlayers(2, gameData);

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Call});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    bbStrategy->setLastAction(GameState::Preflop, {playerBb->getId(), ActionType::Check});

    myHandFsm->runGameLoop();

    EXPECT_FALSE(mockUI->potUpdates.empty()) << "UI should receive pot updates through events";
}

/**
 * Test that UI receives chip stack updates
 */
TEST_F(EventDrivenArchitectureTest, DISABLED_UIReceivesChipUpdates)
{
    initializeHandFsmWithPlayers(2, gameData);

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    myHandFsm->runGameLoop();

    // UI should receive chip updates for affected players
    EXPECT_FALSE(mockUI->chipUpdates.empty()) << "UI should receive chip stack updates through events";

    // Verify we got updates for at least one player
    std::set<unsigned> updatedPlayers;
    for (const auto& [playerId, chips] : mockUI->chipUpdates)
    {
        updatedPlayers.insert(playerId);
    }

    EXPECT_GE(updatedPlayers.size(), 1u) << "At least one player should have chip updates";
}

/**
 * Test error handling - invalid player actions should fire error events
 */
TEST_F(EventDrivenArchitectureTest, EngineFiresErrorEvents)
{
    // Clear events before initializing the game
    mockUI->clear();

    initializeHandFsmWithPlayers(2, gameData);

    // Try to make an invalid action (player not in turn)
    PlayerAction invalidAction;
    invalidAction.playerId = 99; // Non-existent player
    invalidAction.type = ActionType::Fold;
    invalidAction.amount = 0;

    // This should trigger an error event
    myHandFsm->handlePlayerAction(invalidAction);

    // Check that error was captured
    EXPECT_FALSE(mockUI->invalidActions.empty()) << "UI should receive invalid action events";

    if (!mockUI->invalidActions.empty())
    {
        const auto& errorData = mockUI->invalidActions[0];
        EXPECT_EQ(errorData.playerId, 99u) << "Error should contain the invalid player ID";
        EXPECT_FALSE(errorData.reason.empty()) << "Error should contain a descriptive reason";
    }
}

/**
 * Test infinite loop prevention - repeated invalid actions should trigger auto-fold
 */
TEST_F(EventDrivenArchitectureTest, EngineAutoFoldsAfterRepeatedInvalidActions)
{
    // Clear events before initializing the game
    mockUI->clear();

    initializeHandFsmWithPlayers(2, gameData);

    // Get the first player who should act (small blind)
    auto processor = myHandFsm->getActionProcessor();
    ASSERT_NE(processor, nullptr) << "Should have action processor in preflop";

    auto currentPlayer = processor->getNextPlayerToAct(*myHandFsm);
    ASSERT_NE(currentPlayer, nullptr) << "Should have a player to act";

    unsigned playerToAct = currentPlayer->getId();

    // Create an invalid action (trying to check when there's a bet to call)
    PlayerAction invalidAction;
    invalidAction.playerId = playerToAct;
    invalidAction.type = ActionType::Check; // Invalid because big blind was posted
    invalidAction.amount = 0;

    // Submit the same invalid action multiple times
    for (int i = 0; i < 3; ++i) // Exactly MAX_INVALID_ACTIONS (3)
    {
        myHandFsm->handlePlayerAction(invalidAction);
    }

    // One more should trigger auto-fold
    myHandFsm->handlePlayerAction(invalidAction);

    // Check that we received error events
    EXPECT_GE(mockUI->invalidActions.size(), 3u) << "Should receive multiple invalid action events";

    // Check that an engine error was fired (auto-fold notification)
    EXPECT_FALSE(mockUI->engineErrors.empty()) << "Should receive engine error about auto-fold";

    // Check that a player action was eventually fired (the auto-fold)
    EXPECT_FALSE(mockUI->playerActions.empty()) << "Should receive the auto-fold action event";

    if (!mockUI->playerActions.empty())
    {
        const auto& autoFoldAction = mockUI->playerActions.back();
        EXPECT_EQ(autoFoldAction.playerId, playerToAct) << "Auto-fold should be for the problematic player";
        EXPECT_EQ(autoFoldAction.type, ActionType::Fold) << "Auto-action should be a fold";
    }
}

} // namespace pkt::test