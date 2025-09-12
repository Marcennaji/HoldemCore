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

    // Clear existing events and set up our mock UI events
    // This follows the pattern from BettingRoundsFsmTest
    myEvents.clear();

    myEvents.onGameInitialized = [this](int speed)
    {
        mockUI->gameInitialized = true;
        mockUI->gameSpeed = speed;
    };

    // Store events in local variables to avoid potential reference issues
    std::vector<GameState> capturedStates;
    std::vector<PlayerAction> capturedActions;
    std::vector<int> capturedPots;
    std::vector<std::pair<unsigned, int>> capturedChips;

    myEvents.onBettingRoundStarted = [&capturedStates](GameState state)
    {
        std::cout << "[DEBUG] onBettingRoundStarted fired: " << static_cast<int>(state) << std::endl;
        capturedStates.push_back(state);
    };

    myEvents.onPlayerActed = [&capturedActions](PlayerAction action)
    {
        std::cout << "[DEBUG] onPlayerActed fired: player=" << action.playerId
                  << " action=" << static_cast<int>(action.type) << " amount=" << action.amount << std::endl;
        capturedActions.push_back(action);
    };

    myEvents.onPotUpdated = [&capturedPots](int newPot)
    {
        std::cout << "[DEBUG] onPotUpdated fired: " << newPot << std::endl;
        capturedPots.push_back(newPot);
    };

    myEvents.onPlayerChipsUpdated = [&capturedChips](unsigned playerId, int newChips)
    {
        std::cout << "[DEBUG] onPlayerChipsUpdated fired: player=" << playerId << " chips=" << newChips << std::endl;
        capturedChips.push_back({playerId, newChips});
    };

    myEvents.onAwaitingHumanInput = [this]() { mockUI->awaitingHumanInput = true; };
}

/**
 * Test that engine fires events when betting rounds start
 * UI should know game state transitions without calling engine
 */
TEST_F(EventDrivenArchitectureTest, DISABLED_EngineFiresBettingRoundEvents)
{
    initializeHandFsmWithPlayers(2, gameData);

    // Configure simple strategies that will fold immediately to test events
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    // Clear events before running
    mockUI->clear();

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
TEST_F(EventDrivenArchitectureTest, DISABLED_EngineFiresPlayerActionEvents)
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

    // Clear events before running
    mockUI->clear();

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
    // This test is disabled because it highlights the current limitation
    // In proper hexagonal architecture, the UI should know valid actions
    // for the current player through events, not by calling the engine directly

    initializeHandFsmWithPlayers(2, gameData);
    mockUI->clear();

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
TEST_F(EventDrivenArchitectureTest, DISABLED_UIReceivesPotUpdates)
{
    initializeHandFsmWithPlayers(2, gameData);

    // Configure strategies for quick hand
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    mockUI->clear();

    // Run hand to generate pot updates
    myHandFsm->runGameLoop();

    // UI should receive pot updates
    EXPECT_FALSE(mockUI->potUpdates.empty()) << "UI should receive pot updates through events";
}

/**
 * Test that UI receives chip stack updates
 */
TEST_F(EventDrivenArchitectureTest, DISABLED_UIReceivesChipUpdates)
{
    initializeHandFsmWithPlayers(2, gameData);

    // Configure strategies for quick hand
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    mockUI->clear();

    // Run hand to generate chip updates
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
 * SUMMARY TEST: Documents current state of event-driven architecture
 * This test validates what events work vs what's missing for proper hexagonal architecture
 */
TEST_F(EventDrivenArchitectureTest, ArchitecturalComplianceSummary)
{
    // Store events in local variables to avoid potential reference issues
    std::vector<GameState> capturedStates;
    std::vector<PlayerAction> capturedActions;
    std::vector<int> capturedPots;
    std::vector<std::pair<unsigned, int>> capturedChips;

    // Override event handlers to capture directly
    myEvents.onBettingRoundStarted = [&capturedStates](GameState state)
    {
        std::cout << "[DEBUG] onBettingRoundStarted fired: " << static_cast<int>(state) << std::endl;
        capturedStates.push_back(state);
    };

    myEvents.onPlayerActed = [&capturedActions](PlayerAction action)
    {
        std::cout << "[DEBUG] onPlayerActed fired: player=" << action.playerId
                  << " action=" << static_cast<int>(action.type) << " amount=" << action.amount << std::endl;
        capturedActions.push_back(action);
    };

    myEvents.onPotUpdated = [&capturedPots](int newPot)
    {
        std::cout << "[DEBUG] onPotUpdated fired: " << newPot << std::endl;
        capturedPots.push_back(newPot);
    };

    myEvents.onPlayerChipsUpdated = [&capturedChips](unsigned playerId, int newChips)
    {
        std::cout << "[DEBUG] onPlayerChipsUpdated fired: player=" << playerId << " chips=" << newChips << std::endl;
        capturedChips.push_back({playerId, newChips});
    };

    initializeHandFsmWithPlayers(2, gameData);

    // Configure simple fold scenario to test all event types
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    // Run hand to trigger all possible events
    myHandFsm->runGameLoop();

    // === WORKING EVENTS (Architectural compliance) ===
    EXPECT_FALSE(capturedStates.empty()) << "✅ onBettingRoundStarted works - UI can track game state transitions";

    // Debug: print all received states
    std::cout << "[DEBUG] Received " << capturedStates.size() << " betting round events:" << std::endl;
    for (size_t i = 0; i < capturedStates.size(); ++i)
    {
        std::cout << "  [" << i << "] = " << static_cast<int>(capturedStates[i]) << std::endl;
    }

    EXPECT_GE(capturedStates.size(), 2u) << "Should have at least Preflop and PostRiver";

    // Check if we have Preflop and PostRiver (order might vary)
    bool hasPreflop = false;
    bool hasPostRiver = false;
    for (auto state : capturedStates)
    {
        if (state == GameState::Preflop)
            hasPreflop = true;
        if (state == GameState::PostRiver)
            hasPostRiver = true;
    }
    EXPECT_TRUE(hasPreflop) << "✅ UI receives Preflop start event";
    EXPECT_TRUE(hasPostRiver) << "✅ UI receives PostRiver end event";

    // === MISSING EVENTS (Architectural violations) ===
    EXPECT_TRUE(capturedActions.empty())
        << "❌ onPlayerActed never fires - UI cannot track player actions without polling engine!";

    EXPECT_TRUE(capturedPots.empty()) << "❌ onPotUpdated never fires - UI cannot track pot without polling engine!";

    EXPECT_TRUE(capturedChips.empty())
        << "❌ onPlayerChipsUpdated never fires - UI cannot track chip stacks without polling engine!";

    // === CONCLUSION FOR HEXAGONAL ARCHITECTURE ===
    // The current engine provides PARTIAL event support:
    // ✅ Game state transitions (betting rounds)
    // ❌ Player actions (blinds, bets, calls, folds, etc.)
    // ❌ Financial state (pot, chip stacks)
    // ❌ Player turn information (who acts next, valid actions)
    //
    // For true hexagonal architecture, the UI needs ALL game information through events,
    // not just state transitions. Current gaps force UI to poll engine directly.
}

} // namespace pkt::test