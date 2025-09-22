// HoldemCore — Unit tests for Session
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include <core/engine/model/GameData.h>
#include <core/engine/model/StartData.h>
#include <core/services/ServiceContainer.h>
#include <core/session/Session.h>
#include <core/player/Player.h>
#include <core/player/strategy/HumanStrategy.h>
#include <core/player/strategy/LooseAggressiveBotStrategy.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include "SessionTestMocks.h"

using namespace pkt::core;
using namespace pkt::core::test;

// Helper class to access protected methods for testing
class SessionTestHelper : public Session
{
  public:
    SessionTestHelper(const GameEvents& events) : Session(events) {}
    SessionTestHelper(const GameEvents& events, std::shared_ptr<EngineFactory> factory) : Session(events, factory) {}
    SessionTestHelper(const GameEvents& events, std::shared_ptr<ServiceContainer> serviceContainer)
        : Session(events, serviceContainer)
    {
    }

    // Expose protected methods for testing
    using Session::createBoard;
    using Session::ensureEngineFactoryInitialized;
    using Session::ensureServiceContainerInitialized;
    using Session::fireGameInitializedEvent;
    using Session::validateGameParameters;
    using Session::validatePlayerConfiguration;
};

class SessionUnitTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // Setup common test data
        events = GameEvents{};
        testStartData = StartData{};
        testTableProfile = TableProfile{};
    }

    GameEvents events;
    StartData testStartData;
    TableProfile testTableProfile;
};

// Test 1: Constructor with just events
TEST_F(SessionUnitTest, ConstructorWithEventsOnlyCreatesValidSession)
{
    TestableSession session(events);
    EXPECT_FALSE(session.hasEngineFactory()) << "Session should not have engine factory when created with events only";
}

// Test 2: Constructor with events and factory
TEST_F(SessionUnitTest, ConstructorWithEventsAndFactoryCreatesValidSession)
{
    auto factory = std::make_shared<EngineFactory>(events);
    TestableSession session(events, factory);
    EXPECT_TRUE(session.hasEngineFactory()) << "Session should have engine factory when provided in constructor";
}

// Test 2b: Constructor with events and ServiceContainer
TEST_F(SessionUnitTest, ConstructorWithEventsAndServiceContainerCreatesValidSession)
{
    auto serviceContainer = std::make_shared<AppServiceContainer>();
    SessionTestHelper session(events, serviceContainer);

    // The session should be successfully created with the service container
    // We can test this by calling ensureServiceContainerInitialized() which should not throw
    EXPECT_NO_THROW(session.ensureServiceContainerInitialized()) << "Session should have valid service container";
}

// Test 3: Test exception handling for factory methods
TEST_F(SessionUnitTest, FactoryMethodsWithoutEngineFactoryThrowsException)
{
    TestableSession session(events);

    // These methods should throw when no engine factory is available
    EXPECT_THROW(session.testCreateBoard(testStartData), std::exception)
        << "createBoard should throw when no engine factory is available";
}

// Test 4: Test strategy assigner creation
TEST_F(SessionUnitTest, CreateStrategyAssignerWithValidParamsReturnsAssigner)
{
    TestableSession session(events);
    auto assigner = session.testCreateStrategyAssigner(testTableProfile, 3);
    EXPECT_NE(assigner, nullptr) << "Strategy assigner should be created successfully";
}

// Test 5: Test player factory creation
TEST_F(SessionUnitTest, CreatePlayerFactoryWithValidParamsReturnsFactory)
{
    TestableSession session(events);
    auto assigner = session.testCreateStrategyAssigner(testTableProfile, 3);
    auto factory = session.testCreatePlayerFactory(events, assigner.get());
    EXPECT_NE(factory, nullptr) << "Player factory should be created successfully";
}

// Test 6: Test Session basic instantiation
TEST_F(SessionUnitTest, SessionInstantiationBasicSucceeds)
{
    EXPECT_NO_THROW({ Session session(events); }) << "Basic Session instantiation should not throw";
}

// Test 7: Test Session with engine factory instantiation
TEST_F(SessionUnitTest, SessionInstantiationWithFactorySucceeds)
{
    auto factory = std::make_shared<EngineFactory>(events);
    EXPECT_NO_THROW({ Session session(events, factory); }) << "Session instantiation with factory should not throw";
}

// Test 8: Test copy/move semantics
TEST_F(SessionUnitTest, SessionCopyConstructorIsDeleted)
{
    // Verify that Session is not copyable (good for RAII)
    static_assert(!std::is_copy_constructible_v<Session>, "Session should not be copy constructible");
    static_assert(!std::is_copy_assignable_v<Session>, "Session should not be copy assignable");
}

// Test 9: Basic factory method functionality test
TEST_F(SessionUnitTest, FactoryMethodsCreateComponentsReturnsValidPointers)
{
    TestableSession session(events);

    // Test strategy assigner creation
    auto assigner = session.testCreateStrategyAssigner(testTableProfile, 2);
    ASSERT_NE(assigner, nullptr) << "Strategy assigner should not be null";

    // Test player factory creation with the assigner
    auto playerFactory = session.testCreatePlayerFactory(events, assigner.get());
    ASSERT_NE(playerFactory, nullptr) << "Player factory should not be null";
}

// Test 10: Parameter validation tests
TEST_F(SessionUnitTest, ValidateGameParametersWithValidDataDoesNotThrow)
{
    SessionTestHelper session(events);
    GameData validGameData;
    validGameData.startMoney = 1000;

    StartData validStartData;
    validStartData.numberOfPlayers = 4;
    validStartData.startDealerPlayerId = 0;

    EXPECT_NO_THROW(session.validateGameParameters(validGameData, validStartData))
        << "Valid parameters should not throw";
}

TEST_F(SessionUnitTest, ValidateGameParametersWithInvalidPlayerCountThrows)
{
    SessionTestHelper session(events);
    GameData gameData;
    gameData.startMoney = 1000;

    // Test too few players
    StartData invalidStartData;
    invalidStartData.numberOfPlayers = 1;
    invalidStartData.startDealerPlayerId = 0;

    EXPECT_THROW(session.validateGameParameters(gameData, invalidStartData), std::invalid_argument)
        << "Too few players should throw";

    // Test too many players
    invalidStartData.numberOfPlayers = 11;
    EXPECT_THROW(session.validateGameParameters(gameData, invalidStartData), std::invalid_argument)
        << "Too many players should throw";
}

TEST_F(SessionUnitTest, ValidateGameParametersWithInvalidMoneyThrows)
{
    SessionTestHelper session(events);
    GameData invalidGameData;
    invalidGameData.startMoney = 0; // Invalid

    StartData startData;
    startData.numberOfPlayers = 4;
    startData.startDealerPlayerId = 0;

    EXPECT_THROW(session.validateGameParameters(invalidGameData, startData), std::invalid_argument)
        << "Zero start money should throw";
}

TEST_F(SessionUnitTest, ValidateGameParametersWithInvalidDealerIdThrows)
{
    SessionTestHelper session(events);
    GameData gameData;
    gameData.startMoney = 1000;

    StartData invalidStartData;
    invalidStartData.numberOfPlayers = 4;
    invalidStartData.startDealerPlayerId = 5; // Invalid - beyond player range

    EXPECT_THROW(session.validateGameParameters(gameData, invalidStartData), std::invalid_argument)
        << "Invalid dealer ID should throw";
}

// Test 11: Event firing tests
TEST_F(SessionUnitTest, FireGameInitializedEventWithValidHandlerCallsEvent)
{
    bool eventCalled = false;
    int receivedSpeed = 0;

    GameEvents testEvents;
    testEvents.onGameInitialized = [&eventCalled, &receivedSpeed](int speed)
    {
        eventCalled = true;
        receivedSpeed = speed;
    };

    SessionTestHelper session(testEvents);
    session.fireGameInitializedEvent(100);

    EXPECT_TRUE(eventCalled) << "Event should have been called";
    EXPECT_EQ(receivedSpeed, 100) << "Event should receive correct speed";
}

TEST_F(SessionUnitTest, FireGameInitializedEventWithNullHandlerDoesNotCrash)
{
    GameEvents testEvents;
    testEvents.onGameInitialized = nullptr; // Null handler

    SessionTestHelper session(testEvents);

    EXPECT_NO_THROW(session.fireGameInitializedEvent(50)) << "Null event handler should not crash";
}

// Test 12: Engine factory initialization tests
TEST_F(SessionUnitTest, EnsureEngineFactoryInitializedWithoutFactoryCreatesFactory)
{
    SessionTestHelper session(events);

    // Before initialization, createBoard should throw
    StartData testData{};
    testData.numberOfPlayers = 2;
    testData.startDealerPlayerId = 0;
    EXPECT_THROW(session.createBoard(testData), std::runtime_error) << "Should throw without factory";

    // After initialization, createBoard should work
    session.ensureEngineFactoryInitialized();
    EXPECT_NO_THROW(session.createBoard(testData)) << "Should not throw after factory initialization";
}

TEST_F(SessionUnitTest, EnsureEngineFactoryInitializedWithExistingFactoryDoesNotReplace)
{
    auto factory = std::make_shared<EngineFactory>(events);
    TestableSession session(events, factory);
    EXPECT_TRUE(session.hasEngineFactory()) << "Should start with factory";

    // Test that it still works after calling the method
    SessionTestHelper testSession(events, factory);
    testSession.ensureEngineFactoryInitialized();

    StartData testData{};
    testData.numberOfPlayers = 2;
    testData.startDealerPlayerId = 0;
    EXPECT_NO_THROW(testSession.createBoard(testData)) << "Should still work after calling ensure";
}

// Tests for validatePlayerConfiguration
TEST_F(SessionUnitTest, ValidatePlayerConfigurationWithValidMixedPlayersSucceeds)
{
    SessionTestHelper session(events);
    
    // Create a valid player list: 1 human (ID 0) + 2 bots (IDs 1,2)
    auto playersList = std::make_shared<std::list<std::shared_ptr<player::Player>>>();
    
    // Create human player with ID 0
    auto humanPlayer = std::make_shared<player::Player>(events, 0, "Human", 1000);
    auto humanStrategy = std::make_unique<player::HumanStrategy>(events);
    humanPlayer->setStrategy(std::move(humanStrategy));
    playersList->push_back(humanPlayer);
    
    // Create bot players with IDs 1 and 2
    for (int i = 1; i <= 2; ++i) {
        auto botPlayer = std::make_shared<player::Player>(events, i, "Bot_" + std::to_string(i), 1000);
        auto botStrategy = std::make_unique<player::LooseAggressiveBotStrategy>();
        botPlayer->setStrategy(std::move(botStrategy));
        playersList->push_back(botPlayer);
    }
    
    // Should not throw
    EXPECT_NO_THROW(session.validatePlayerConfiguration(playersList));
}

TEST_F(SessionUnitTest, ValidatePlayerConfigurationWithEmptyListThrows)
{
    SessionTestHelper session(events);
    
    auto emptyPlayersList = std::make_shared<std::list<std::shared_ptr<player::Player>>>();
    
    EXPECT_THROW(session.validatePlayerConfiguration(emptyPlayersList), std::runtime_error);
}

TEST_F(SessionUnitTest, ValidatePlayerConfigurationWithNullListThrows)
{
    SessionTestHelper session(events);
    
    pkt::core::player::PlayerList nullPlayersList = nullptr;
    
    EXPECT_THROW(session.validatePlayerConfiguration(nullPlayersList), std::runtime_error);
}

TEST_F(SessionUnitTest, ValidatePlayerConfigurationWithNoHumanPlayerThrows)
{
    SessionTestHelper session(events);
    
    // Create a list with only bot players
    auto playersList = std::make_shared<std::list<std::shared_ptr<player::Player>>>();
    
    for (int i = 0; i < 3; ++i) {
        auto botPlayer = std::make_shared<player::Player>(events, i, "Bot_" + std::to_string(i), 1000);
        auto botStrategy = std::make_unique<player::LooseAggressiveBotStrategy>();
        botPlayer->setStrategy(std::move(botStrategy));
        playersList->push_back(botPlayer);
    }
    
    EXPECT_THROW(session.validatePlayerConfiguration(playersList), std::runtime_error);
}

TEST_F(SessionUnitTest, ValidatePlayerConfigurationWithMultipleHumanPlayersThrows)
{
    SessionTestHelper session(events);
    
    // Create a list with 2 human players and 1 bot
    auto playersList = std::make_shared<std::list<std::shared_ptr<player::Player>>>();
    
    // Create 2 human players
    for (int i = 0; i < 2; ++i) {
        auto humanPlayer = std::make_shared<player::Player>(events, i, "Human_" + std::to_string(i), 1000);
        auto humanStrategy = std::make_unique<player::HumanStrategy>(events);
        humanPlayer->setStrategy(std::move(humanStrategy));
        playersList->push_back(humanPlayer);
    }
    
    // Create 1 bot player
    auto botPlayer = std::make_shared<player::Player>(events, 2, "Bot_2", 1000);
    auto botStrategy = std::make_unique<player::LooseAggressiveBotStrategy>();
    botPlayer->setStrategy(std::move(botStrategy));
    playersList->push_back(botPlayer);
    
    EXPECT_THROW(session.validatePlayerConfiguration(playersList), std::runtime_error);
}

TEST_F(SessionUnitTest, ValidatePlayerConfigurationWithHumanPlayerNotIdZeroThrows)
{
    SessionTestHelper session(events);
    
    // Create a list with human player having ID 1 instead of 0
    auto playersList = std::make_shared<std::list<std::shared_ptr<player::Player>>>();
    
    // Create bot player with ID 0
    auto botPlayer = std::make_shared<player::Player>(events, 0, "Bot_0", 1000);
    auto botStrategy = std::make_unique<player::LooseAggressiveBotStrategy>();
    botPlayer->setStrategy(std::move(botStrategy));
    playersList->push_back(botPlayer);
    
    // Create human player with ID 1 (wrong!)
    auto humanPlayer = std::make_shared<player::Player>(events, 1, "Human", 1000);
    auto humanStrategy = std::make_unique<player::HumanStrategy>(events);
    humanPlayer->setStrategy(std::move(humanStrategy));
    playersList->push_back(humanPlayer);
    
    EXPECT_THROW(session.validatePlayerConfiguration(playersList), std::runtime_error);
}