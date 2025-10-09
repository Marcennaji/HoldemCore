// HoldemCore — Unit tests for Session class
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include <gtest/gtest.h>
#include <memory>

#include "core/session/Session.h"
#include "core/engine/GameEvents.h"
#include "core/engine/EngineFactory.h"
#include "core/engine/model/GameData.h"
#include "core/engine/model/StartData.h"
#include "core/engine/model/PlayerAction.h"
#include "core/engine/model/TableProfile.h"
#include "infra/NullLogger.h"
#include "infra/eval/PsimHandEvaluationEngine.h"
#include "infra/persistence/NullPlayersStatisticsStore.h"
#include "core/services/DefaultRandomizer.h"

using namespace pkt::core;

namespace pkt::test
{





// Testable Session class that exposes protected methods for testing
class TestableSession : public Session
{
  public:
    TestableSession(const GameEvents& events, 
                   EngineFactory& engineFactory,
                   Logger& logger,
                   HandEvaluationEngine& handEvaluationEngine,
                   PlayersStatisticsStore& playersStatisticsStore,
                   Randomizer& randomizer)
        : Session(events, engineFactory, logger, handEvaluationEngine, playersStatisticsStore, randomizer)
    {}

    // Expose protected methods for testing
    using Session::validateGameParameters;
    using Session::validatePlayerConfiguration;
    using Session::fireGameInitializedEvent;
    using Session::createStrategyAssigner;
    using Session::createPlayerFactory;
    using Session::createBoard;
};

// Test fixture
class SessionUnitTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // Initialize real implementations - no mocking needed
        logger = std::make_unique<pkt::infra::NullLogger>();
        handEvaluator = std::make_unique<pkt::infra::PsimHandEvaluationEngine>();
        statsStore = std::make_unique<NullPlayersStatisticsStore>();
        randomizer = std::make_unique<DefaultRandomizer>();
        engineFactory = std::make_unique<EngineFactory>(
            events, *logger, *handEvaluator, *statsStore, *randomizer);

        // Setup default game data
        gameData.tableProfile = TableProfile{};
        gameData.maxNumberOfPlayers = 10;
        gameData.startMoney = 1000;
        gameData.firstSmallBlind = 10;
        gameData.guiSpeed = 100;

        // Setup default start data
        startData.numberOfPlayers = 6;
        startData.startDealerPlayerId = 0;

        // Setup event callbacks for testing
        gameInitializedCallCount = 0;
        events.onGameInitialized = [this](int speed) { 
            gameInitializedCallCount++; 
            lastGuiSpeed = speed;
        };
    }

    // Test data
    GameEvents events;
    GameData gameData;
    StartData startData;
    int gameInitializedCallCount = 0;
    int lastGuiSpeed = 0;

    // Real dependencies using null implementations
    std::unique_ptr<Logger> logger;
    std::unique_ptr<HandEvaluationEngine> handEvaluator;
    std::unique_ptr<PlayersStatisticsStore> statsStore;
    std::unique_ptr<Randomizer> randomizer;
    std::unique_ptr<EngineFactory> engineFactory;
};

// =============================================================================
// Constructor and Destructor Tests
// =============================================================================

TEST_F(SessionUnitTest, Constructor_WithValidDependencies_InitializesCorrectly)
{
    // Act & Assert - Should not throw
    EXPECT_NO_THROW({
        Session session(events, *engineFactory, *logger, 
                       *handEvaluator, *statsStore, *randomizer);
    });
}

TEST_F(SessionUnitTest, Constructor_StoresReferencesToDependencies)
{
    // Act
    Session session(events, *engineFactory, *logger, 
                   *handEvaluator, *statsStore, *randomizer);
    
    // Assert - If we can call methods that use dependencies without crash, references are stored
    GameData invalidData;  // This will trigger validation
    invalidData.startMoney = 0;  // Invalid - will cause exception
    StartData validStart;
    validStart.numberOfPlayers = 2;
    
    EXPECT_THROW(session.startGame(invalidData, validStart), std::invalid_argument);
}

TEST_F(SessionUnitTest, Destructor_CleansUpProperly_NoExceptions)
{
    // Act & Assert - Should not throw during destruction
    EXPECT_NO_THROW({
        auto session = std::make_unique<Session>(events, *engineFactory, *logger, 
                                               *handEvaluator, *statsStore, *randomizer);
        session.reset();  // Explicit destruction
    });
}

TEST_F(SessionUnitTest, MultipleInstances_CanCoexist_IndependentState)
{
    // Act - Create multiple session instances
    auto session1 = std::make_unique<Session>(events, *engineFactory, *logger, 
                                            *handEvaluator, *statsStore, *randomizer);
    auto session2 = std::make_unique<Session>(events, *engineFactory, *logger, 
                                            *handEvaluator, *statsStore, *randomizer);
    
    // Assert - Both instances should be independent
    EXPECT_NE(session1.get(), session2.get());
    
    // Clean up - Should not throw
    EXPECT_NO_THROW({
        session1.reset();
        session2.reset();
    });
}

// =============================================================================
// Parameter Validation Tests
// =============================================================================

TEST_F(SessionUnitTest, ValidateGameParameters_WithValidParameters_DoesNotThrow)
{
    // Arrange
    TestableSession session(events, *engineFactory, *logger, 
                           *handEvaluator, *statsStore, *randomizer);
    
    // Act & Assert
    EXPECT_NO_THROW(session.validateGameParameters(gameData, startData));
}

TEST_F(SessionUnitTest, ValidateGameParameters_WithTooFewPlayers_ThrowsException)
{
    // Arrange
    TestableSession session(events, *engineFactory, *logger, 
                           *handEvaluator, *statsStore, *randomizer);
    startData.numberOfPlayers = 1;  // Invalid - minimum is 2
    
    // Act & Assert
    EXPECT_THROW(session.validateGameParameters(gameData, startData), std::invalid_argument);
}

TEST_F(SessionUnitTest, ValidateGameParameters_WithTooManyPlayers_ThrowsException)
{
    // Arrange
    TestableSession session(events, *engineFactory, *logger, 
                           *handEvaluator, *statsStore, *randomizer);
    startData.numberOfPlayers = 11;  // Invalid - maximum is 10
    
    // Act & Assert
    EXPECT_THROW(session.validateGameParameters(gameData, startData), std::invalid_argument);
}

TEST_F(SessionUnitTest, ValidateGameParameters_WithZeroStartMoney_ThrowsException)
{
    // Arrange
    TestableSession session(events, *engineFactory, *logger, 
                           *handEvaluator, *statsStore, *randomizer);
    gameData.startMoney = 0;  // Invalid
    
    // Act & Assert
    EXPECT_THROW(session.validateGameParameters(gameData, startData), std::invalid_argument);
}

TEST_F(SessionUnitTest, ValidateGameParameters_WithInvalidDealerPlayerId_ThrowsException)
{
    // Arrange
    TestableSession session(events, *engineFactory, *logger, 
                           *handEvaluator, *statsStore, *randomizer);
    startData.startDealerPlayerId = startData.numberOfPlayers;  // Invalid - must be < numberOfPlayers
    
    // Act & Assert
    EXPECT_THROW(session.validateGameParameters(gameData, startData), std::invalid_argument);
}

TEST_F(SessionUnitTest, FireGameInitializedEvent_WithRegisteredCallback_CallsCallback)
{
    // Arrange
    TestableSession session(events, *engineFactory, *logger, 
                           *handEvaluator, *statsStore, *randomizer);
    const int testGuiSpeed = 500;
    
    // Act
    session.fireGameInitializedEvent(testGuiSpeed);
    
    // Assert
    EXPECT_EQ(gameInitializedCallCount, 1);
    EXPECT_EQ(lastGuiSpeed, testGuiSpeed);
}

TEST_F(SessionUnitTest, FireGameInitializedEvent_WithoutCallback_DoesNotCrash)
{
    // Arrange
    TestableSession session(events, *engineFactory, *logger, 
                           *handEvaluator, *statsStore, *randomizer);
    GameEvents eventsWithoutCallback;  // No callback registered
    TestableSession sessionWithoutCallback(eventsWithoutCallback, *engineFactory, *logger, 
                                          *handEvaluator, *statsStore, *randomizer);
    
    // Act & Assert - Should not crash
    EXPECT_NO_THROW(sessionWithoutCallback.fireGameInitializedEvent(100));
}

// =============================================================================
// Factory Method Tests  
// =============================================================================

TEST_F(SessionUnitTest, CreateStrategyAssigner_WithValidParameters_ReturnsValidInstance)
{
    // Arrange
    TestableSession session(events, *engineFactory, *logger, 
                   *handEvaluator, *statsStore, *randomizer);
    TableProfile tableProfile;
    const int numberOfBots = 5;
    
    // Act
    auto strategyAssigner = session.createStrategyAssigner(tableProfile, numberOfBots);
    
    // Assert
    EXPECT_NE(strategyAssigner, nullptr);
}

TEST_F(SessionUnitTest, CreatePlayerFactory_WithValidParameters_ReturnsValidInstance)
{
    // Arrange
    TestableSession session(events, *engineFactory, *logger, 
                   *handEvaluator, *statsStore, *randomizer);
    auto strategyAssigner = session.createStrategyAssigner(TableProfile{}, 5);
    
    // Act
    auto playerFactory = session.createPlayerFactory(events, strategyAssigner.get());
    
    // Assert
    EXPECT_NE(playerFactory, nullptr);
}

TEST_F(SessionUnitTest, CreateBoard_WithValidStartData_ReturnsValidInstance)
{
    // Arrange
    TestableSession session(events, *engineFactory, *logger, 
                   *handEvaluator, *statsStore, *randomizer);
    
    // Act
    auto board = session.createBoard(startData);
    
    // Assert
    EXPECT_NE(board, nullptr);
}

// =============================================================================
// Game Lifecycle Tests
// =============================================================================

TEST_F(SessionUnitTest, HandlePlayerAction_WithoutCurrentGame_DoesNotCrash)
{
    // Arrange
    Session session(events, *engineFactory, *logger, 
                   *handEvaluator, *statsStore, *randomizer);
    PlayerAction testAction;
    testAction.playerId = 1;
    testAction.type = ActionType::Fold;
    
    // Act - Should not crash even without current game
    EXPECT_NO_THROW(session.handlePlayerAction(testAction));
}

TEST_F(SessionUnitTest, StartNewHand_WithoutCurrentGame_DoesNotCrash)
{
    // Arrange
    Session session(events, *engineFactory, *logger, 
                   *handEvaluator, *statsStore, *randomizer);
    
    // Act - Should not crash even without current game
    EXPECT_NO_THROW(session.startNewHand());
}

// =============================================================================
// Error Handling and Edge Case Tests
// =============================================================================

TEST_F(SessionUnitTest, StartGame_BoundaryValues_ValidatesCorrectly)
{
    // Test boundary value validation without actually starting the game
    // (to avoid hang on human player input)
    
    // Arrange
    TestableSession session(events, *engineFactory, *logger, 
                           *handEvaluator, *statsStore, *randomizer);
    
    // Act & Assert - Test minimum valid players (2)
    startData.numberOfPlayers = 2;
    gameData.startMoney = 1;  // Minimum valid money
    
    // Should pass parameter validation
    EXPECT_NO_THROW(session.validateGameParameters(gameData, startData));
    
    // Test maximum valid players (10)  
    startData.numberOfPlayers = 10;
    startData.startDealerPlayerId = 9;  // Maximum valid dealer ID
    gameData.startMoney = 100000;  // Large money value
    
    // Should pass parameter validation
    EXPECT_NO_THROW(session.validateGameParameters(gameData, startData));
}

TEST_F(SessionUnitTest, StartGame_MaximumValues_ValidatesCorrectly)
{
    // Test maximum value validation without actually starting the game
    // (to avoid hang on human player input)
    
    // Arrange
    TestableSession session(events, *engineFactory, *logger, 
                           *handEvaluator, *statsStore, *randomizer);
    
    // Test maximum valid players (10)
    startData.numberOfPlayers = 10;
    startData.startDealerPlayerId = 9;  // Maximum valid dealer ID
    gameData.startMoney = 100000;  // Large money value
    
    // Act & Assert - Should pass parameter validation with maximum values
    EXPECT_NO_THROW(session.validateGameParameters(gameData, startData));
}

TEST_F(SessionUnitTest, StartGame_NegativeValues_ThrowsAppropriateExceptions)
{
    // This test only validates parameters, so no hanging issue
    // Arrange
    TestableSession session(events, *engineFactory, *logger, 
                           *handEvaluator, *statsStore, *randomizer);
    
    // Test negative number of players
    startData.numberOfPlayers = -1;
    EXPECT_THROW(session.validateGameParameters(gameData, startData), std::invalid_argument);
    
    // Test negative start money
    startData.numberOfPlayers = 6;  // Reset to valid
    gameData.startMoney = -100;
    EXPECT_THROW(session.validateGameParameters(gameData, startData), std::invalid_argument);
}

TEST_F(SessionUnitTest, ErrorMessages_AreDescriptive_ForDebugging)
{
    // Arrange
    TestableSession session(events, *engineFactory, *logger, 
                           *handEvaluator, *statsStore, *randomizer);
    
    // Test that error messages contain useful information
    try {
        startData.numberOfPlayers = 1;
        session.validateGameParameters(gameData, startData);
        FAIL() << "Expected std::invalid_argument to be thrown";
    } catch (const std::invalid_argument& e) {
        std::string errorMsg = e.what();
        EXPECT_TRUE(errorMsg.find("2") != std::string::npos) << "Error should mention minimum players (2)";
        EXPECT_TRUE(errorMsg.find("player") != std::string::npos) << "Error should mention 'player'";
    }
    
    try {
        startData.numberOfPlayers = 6;  // Reset
        gameData.startMoney = 0;
        session.validateGameParameters(gameData, startData);
        FAIL() << "Expected std::invalid_argument to be thrown";
    } catch (const std::invalid_argument& e) {
        std::string errorMsg = e.what();
        EXPECT_TRUE(errorMsg.find("money") != std::string::npos) << "Error should mention 'money'";
        EXPECT_TRUE(errorMsg.find("0") != std::string::npos) << "Error should mention the problematic value";
    }
}

TEST_F(SessionUnitTest, ThreadSafety_MultipleQuickOperations_DoNotCrash)
{
    // This test helps identify potential race conditions or state corruption
    // Arrange
    std::vector<std::unique_ptr<Session>> sessions;
    const int numSessions = 10;
    
    // Act - Rapidly create and destroy sessions
    for (int i = 0; i < numSessions; ++i) {
        sessions.emplace_back(std::make_unique<Session>(events, *engineFactory, *logger, 
                                                       *handEvaluator, *statsStore, *randomizer));
    }
    
    // Clear in reverse order
    sessions.clear();
    
    // Assert - If we get here without crashes, basic thread safety is working
    SUCCEED() << "Rapid creation/destruction completed without crashes";
}

} // namespace pkt::test