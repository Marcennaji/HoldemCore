// tests/e2e/EventDrivenArchitectureTest.cpp

#include "EventDrivenArchitectureTest.h"
#include "common/DeterministicStrategy.h"
#include "core/engine/hand/Hand.h"
#include "core/player/Helpers.h"
#include "core/player/Player.h"
#include "core/player/strategy/HumanStrategy.h"

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
    humanInputRequests.clear();
    handCompletions.clear();
    gameInitialized = false;
    gameSpeed = 0;
}

bool MockUI::canDetermineCurrentPlayer() const
{
    return !playerActions.empty() || !bettingRoundsStarted.empty();
}

bool MockUI::hasValidActionsForCurrentPlayer() const
{
    return !humanInputRequests.empty();
}

bool MockUI::canDisplayGameState() const
{
    return !gameStates.empty() || gameInitialized;
}

unsigned MockUI::getCurrentPlayerFromEvents() const
{
    if (!playerActions.empty())
    {
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
    m_events.clear();

    m_events.onGameInitialized = [this](int speed)
    {
        mockUI->gameInitialized = true;
        mockUI->gameSpeed = speed;
    };

    m_events.onBettingRoundStarted = [this](GameState state) { mockUI->bettingRoundsStarted.push_back(state); };

    m_events.onPlayerActed = [this](PlayerAction action) { mockUI->playerActions.push_back(action); };

    m_events.onPotUpdated = [this](int newPot) { mockUI->potUpdates.push_back(newPot); };

    m_events.onPlayerChipsUpdated = [this](unsigned playerId, int newChips)
    { mockUI->chipUpdates.push_back({playerId, newChips}); };

    m_events.onInvalidPlayerAction = [this](unsigned playerId, PlayerAction invalidAction, std::string reason)
    { mockUI->invalidActions.push_back({playerId, invalidAction, reason}); };

    m_events.onEngineError = [this](std::string errorMessage) { mockUI->engineErrors.push_back(errorMessage); };

    m_events.onAwaitingHumanInput = [this](unsigned playerId, std::vector<ActionType> validActions)
    { mockUI->humanInputRequests.push_back({playerId, validActions}); };

    m_events.onHandCompleted = [this](std::list<unsigned> winnerIds, int totalPot)
    { mockUI->handCompletions.push_back({winnerIds, totalPot}); };
}

/**
 * Test that engine fires events when betting rounds start
 * UI should know game state transitions without calling engine
 */
TEST_F(EventDrivenArchitectureTest, EngineFiresBettingRoundEvents)
{
    initializeHandWithPlayers(2, gameData);

    // Configure simple strategies that will fold immediately to test events
    auto playerSb = getPlayerById(m_actingPlayersList, 0);
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    // Run the hand
    m_hand->runGameLoop();

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
    initializeHandWithPlayers(2, gameData);

    // Configure strategies for a complete hand
    auto playerSb = getPlayerById(m_actingPlayersList, 0);
    auto playerBb = getPlayerById(m_actingPlayersList, 1);

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
    m_hand->runGameLoop();

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
 * Test that UI receives real human input events with calculated valid actions
 * This validates that the engine fires onAwaitingHumanInput with actual valid actions
 * calculated from the current game state when a human player needs to act
 */
TEST_F(EventDrivenArchitectureTest, UIReceivesHumanInputEvents)
{
    initializeHandWithPlayers(2, gameData);

    // Get the first player who should act (small blind)
    auto playerSb = getPlayerById(m_actingPlayersList, 0);
    auto playerBb = getPlayerById(m_actingPlayersList, 1);

    // Set up the test strategy for small blind
    auto testStrategy = std::make_unique<TestHumanEventStrategy>(m_events);
    playerSb->setStrategy(std::move(testStrategy));

    m_hand->runGameLoop();

    // Verify the UI received the human input request with real valid actions
    EXPECT_EQ(mockUI->humanInputRequests.size(), 1u) << "UI should receive human input request";
    EXPECT_EQ(mockUI->humanInputRequests[0].playerId, playerSb->getId()) << "Player ID should match the small blind";

    const auto& receivedActions = mockUI->humanInputRequests[0].validActions;
    EXPECT_FALSE(receivedActions.empty()) << "Should receive actual calculated valid actions";

    // In preflop with big blind posted, small blind should be able to:
    // - Fold (always available)
    // - Call (to match big blind)
    // - Raise (if has enough chips)
    // - Allin (if has chips)
    EXPECT_EQ(receivedActions.size(), 4u) << "Should receive 4 valid actions preflop for small blind";

    EXPECT_TRUE(std::find(receivedActions.begin(), receivedActions.end(), ActionType::Fold) != receivedActions.end())
        << "Fold should always be available";

    EXPECT_TRUE(std::find(receivedActions.begin(), receivedActions.end(), ActionType::Call) != receivedActions.end())
        << "Call should be available to match big blind";

    EXPECT_TRUE(std::find(receivedActions.begin(), receivedActions.end(), ActionType::Allin) != receivedActions.end())
        << "Allin should be available if player has chips";

    EXPECT_TRUE(std::find(receivedActions.begin(), receivedActions.end(), ActionType::Raise) != receivedActions.end())
        << "Raise should be available if player has enough chips";
}

/**
 * Test that UI can track pot changes through events
 */
TEST_F(EventDrivenArchitectureTest, UIReceivesPotUpdates)
{
    initializeHandWithPlayers(2, gameData);

    auto playerSb = getPlayerById(m_actingPlayersList, 0);
    auto playerBb = getPlayerById(m_actingPlayersList, 1);

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
    playerBb->setStrategy(std::move(bbStrategy)); // FIX: Actually assign the strategy!

    m_hand->runGameLoop();

    // Verify no auto-fold occurred (which would indicate test setup problems)
    for (const auto& error : mockUI->engineErrors)
    {
        EXPECT_TRUE(error.find("exceeded maximum invalid actions") == std::string::npos)
            << "Test setup error: Player auto-folded due to invalid actions - " << error;
    }

    EXPECT_FALSE(mockUI->potUpdates.empty()) << "UI should receive pot updates through events";
}

/**
 * Test that UI receives chip stack updates
 */
TEST_F(EventDrivenArchitectureTest, UIReceivesChipUpdates)
{
    initializeHandWithPlayers(2, gameData);

    auto playerSb = getPlayerById(m_actingPlayersList, 0);
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    m_hand->runGameLoop();

    // UI should receive chip updates for affected players
    EXPECT_FALSE(mockUI->chipUpdates.empty()) << "UI should receive chip stack updates through events";

    // Verify we got updates for at least one player
    std::set<unsigned> updatedPlayers;
    for (const auto& [playerId, chips] : mockUI->chipUpdates)
    {
        updatedPlayers.insert(playerId);
    }

    EXPECT_GE(updatedPlayers.size(), 1u) << "At least one player should have chip updates";

    // Expected behavior: Players should have chip updates from both betting and winnings
    // - Small blind posting (chip reduction)
    // - Big blind posting (chip reduction)
    // - Pot distribution to winner (chip increases)
    EXPECT_GE(mockUI->chipUpdates.size(), 2u) << "Should have blind posting and pot distribution updates";

    // Verify both players had chip changes
    EXPECT_EQ(updatedPlayers.size(), 2u) << "Both players should have chip updates in heads-up game";
}

/**
 * Test error handling - invalid player actions should fire error events
 */
TEST_F(EventDrivenArchitectureTest, EngineFiresErrorEvents)
{
    // Clear events before initializing the game
    mockUI->clear();

    initializeHandWithPlayers(2, gameData);

    // Try to make an invalid action (player not in turn)
    PlayerAction invalidAction;
    invalidAction.playerId = 99; // Non-existent player
    invalidAction.type = ActionType::Fold;
    invalidAction.amount = 0;

    // This should trigger an error event
    m_hand->handlePlayerAction(invalidAction);

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

    initializeHandWithPlayers(2, gameData);

    // Get the first player who should act (small blind)
    auto processor = m_hand->getActionProcessor();
    ASSERT_NE(processor, nullptr) << "Should have action processor in preflop";

    auto currentPlayer = processor->getNextPlayerToAct(*m_hand);
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
        m_hand->handlePlayerAction(invalidAction);
    }

    // One more should trigger auto-fold
    m_hand->handlePlayerAction(invalidAction);

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

/**
 * Test that engine fires onHandCompleted event when a hand finishes
 * UI should know when hands complete and who won without calling engine
 */
TEST_F(EventDrivenArchitectureTest, EngineFiresHandCompletedEvent)
{
    initializeHandWithPlayers(3, gameData);

    // Configure strategies so we have a clear winner scenario
    auto playerSb = getPlayerById(m_actingPlayersList, 0);     // Small blind folds
    auto playerBb = getPlayerById(m_actingPlayersList, 1);     // Big blind folds
    auto playerDealer = getPlayerById(m_actingPlayersList, 2); // Dealer wins by default

    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    bbStrategy->setLastAction(GameState::Preflop, {playerBb->getId(), ActionType::Fold});
    playerBb->setStrategy(std::move(bbStrategy));

    // Dealer doesn't need strategy since others fold
    auto dealerStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    dealerStrategy->setLastAction(GameState::Preflop, {playerDealer->getId(), ActionType::Check});
    playerDealer->setStrategy(std::move(dealerStrategy));

    // Clear any existing events and run the hand
    mockUI->clear();
    m_hand->runGameLoop();

    // UI should have received exactly one hand completion event
    ASSERT_FALSE(mockUI->handCompletions.empty()) << "UI should receive onHandCompleted event when hand finishes";

    EXPECT_EQ(mockUI->handCompletions.size(), 1) << "Should receive exactly one hand completion event per hand";

    const auto& handCompletion = mockUI->handCompletions[0];

    // Should have exactly one winner (the dealer who didn't fold)
    EXPECT_FALSE(handCompletion.winnerIds.empty()) << "Hand completion should include winner IDs";

    EXPECT_EQ(handCompletion.winnerIds.size(), 1) << "Should have exactly one winner when others fold";

    if (!handCompletion.winnerIds.empty())
    {
        EXPECT_EQ(handCompletion.winnerIds.front(), playerDealer->getId())
            << "Winner should be the dealer (player who didn't fold)";
    }

    // Should have a reasonable pot size (at least the blinds)
    EXPECT_GT(handCompletion.totalPot, 0) << "Total pot should be greater than 0";

    // Pot should include at least the small and big blinds
    int expectedMinPot = gameData.firstSmallBlind + (gameData.firstSmallBlind * 2); // SB + BB
    EXPECT_GE(handCompletion.totalPot, expectedMinPot)
        << "Pot should at least contain the blinds. Expected >= " << expectedMinPot << ", got "
        << handCompletion.totalPot;
}

/**
 * Test that engine fires card dealing events (onHoleCardsDealt and onBoardCardsDealt)
 * UI should receive events when cards are dealt without polling engine
 */
TEST_F(EventDrivenArchitectureTest, EngineFiresCardDealingEvents)
{
    // Track card dealing events
    std::vector<std::pair<unsigned, HoleCards>> holeCardsDealt;
    std::vector<BoardCards> boardCardsDealt;

    // Add event handlers for card dealing BEFORE initialization so we capture hole cards events
    m_events.onHoleCardsDealt = [&holeCardsDealt](unsigned playerId, HoleCards holeCards)
    { holeCardsDealt.push_back({playerId, holeCards}); };

    m_events.onBoardCardsDealt = [&boardCardsDealt](BoardCards boardCards) { boardCardsDealt.push_back(boardCards); };

    // Initialize AFTER setting up event handlers so we capture hole cards events
    initializeHandWithPlayers(2, gameData);

    // Configure strategies to play through all rounds so we see all board card events
    auto playerSb = getPlayerById(m_actingPlayersList, 0);
    auto playerBb = getPlayerById(m_actingPlayersList, 1);

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
    m_hand->runGameLoop();

    // Verify hole cards events
    ASSERT_EQ(holeCardsDealt.size(), 2) << "Should receive hole cards events for both players";

    // Check that both players received hole cards
    std::set<unsigned> playersWithHoleCards;
    for (const auto& [playerId, holeCards] : holeCardsDealt)
    {
        playersWithHoleCards.insert(playerId);
        // Each player should receive valid hole cards
        EXPECT_TRUE(holeCards.isValid()) << "Each player should receive valid hole cards";
    }
    EXPECT_EQ(playersWithHoleCards.size(), 2) << "Both players should receive hole cards";

    // Verify board cards events
    ASSERT_EQ(boardCardsDealt.size(), 3) << "Should receive board cards events for Flop, Turn, and River";

    // Check Flop event (3 cards)
    auto flopEvent = std::find_if(boardCardsDealt.begin(), boardCardsDealt.end(),
                                  [](const BoardCards& boardCards) { return boardCards.isFlop(); });
    ASSERT_NE(flopEvent, boardCardsDealt.end()) << "Should receive Flop board cards event";
    EXPECT_EQ(flopEvent->getNumCards(), 3) << "Flop should have exactly 3 cards";
    EXPECT_TRUE(flopEvent->isFlop()) << "Board should be in flop state";

    // Check Turn event (4 cards total)
    auto turnEvent = std::find_if(boardCardsDealt.begin(), boardCardsDealt.end(),
                                  [](const BoardCards& boardCards) { return boardCards.isTurn(); });
    ASSERT_NE(turnEvent, boardCardsDealt.end()) << "Should receive Turn board cards event";
    EXPECT_EQ(turnEvent->getNumCards(), 4) << "Turn should have exactly 4 cards total";
    EXPECT_TRUE(turnEvent->isTurn()) << "Board should be in turn state";

    // Check River event (5 cards total)
    auto riverEvent = std::find_if(boardCardsDealt.begin(), boardCardsDealt.end(),
                                   [](const BoardCards& boardCards) { return boardCards.isRiver(); });
    ASSERT_NE(riverEvent, boardCardsDealt.end()) << "Should receive River board cards event";
    EXPECT_EQ(riverEvent->getNumCards(), 5) << "River should have exactly 5 cards total";
    EXPECT_TRUE(riverEvent->isRiver()) << "Board should be in river state";
}

} // namespace pkt::test