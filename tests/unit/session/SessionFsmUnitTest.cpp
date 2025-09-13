// PokerTraining — Unit tests for SessionFsm
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include <core/engine/model/GameData.h>
#include <core/engine/model/StartData.h>
#include <core/session/SessionFsm.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include "SessionFsmTestMocks.h"

using namespace pkt::core;
using namespace pkt::core::test;

// Helper class to access protected methods for testing
class SessionFsmTestHelper : public SessionFsm
{
  public:
    SessionFsmTestHelper(const GameEvents& events) : SessionFsm(events) {}
    SessionFsmTestHelper(const GameEvents& events, std::shared_ptr<EngineFactory> factory) : SessionFsm(events, factory)
    {
    }

    // Expose protected methods for testing
    using SessionFsm::createBoard;
    using SessionFsm::ensureEngineFactoryInitialized;
    using SessionFsm::fireGameInitializedEvent;
    using SessionFsm::validateGameParameters;
};

class SessionFsmUnitTest : public ::testing::Test
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
TEST_F(SessionFsmUnitTest, ConstructorWithEventsOnlyCreatesValidSession)
{
    TestableSessionFsm session(events);
    EXPECT_FALSE(session.hasEngineFactory()) << "Session should not have engine factory when created with events only";
}

// Test 2: Constructor with events and factory
TEST_F(SessionFsmUnitTest, ConstructorWithEventsAndFactoryCreatesValidSession)
{
    auto factory = std::make_shared<EngineFactory>(events);
    TestableSessionFsm session(events, factory);
    EXPECT_TRUE(session.hasEngineFactory()) << "Session should have engine factory when provided in constructor";
}

// Test 3: Test exception handling for factory methods
TEST_F(SessionFsmUnitTest, FactoryMethodsWithoutEngineFactoryThrowsException)
{
    TestableSessionFsm session(events);

    // These methods should throw when no engine factory is available
    EXPECT_THROW(session.testCreateBoard(testStartData), std::exception)
        << "createBoard should throw when no engine factory is available";
}

// Test 4: Test strategy assigner creation
TEST_F(SessionFsmUnitTest, CreateStrategyAssignerWithValidParamsReturnsAssigner)
{
    TestableSessionFsm session(events);
    auto assigner = session.testCreateStrategyAssigner(testTableProfile, 3);
    EXPECT_NE(assigner, nullptr) << "Strategy assigner should be created successfully";
}

// Test 5: Test player factory creation
TEST_F(SessionFsmUnitTest, CreatePlayerFactoryWithValidParamsReturnsFactory)
{
    TestableSessionFsm session(events);
    auto assigner = session.testCreateStrategyAssigner(testTableProfile, 3);
    auto factory = session.testCreatePlayerFactory(events, assigner.get());
    EXPECT_NE(factory, nullptr) << "Player factory should be created successfully";
}

// Test 6: Test SessionFsm basic instantiation
TEST_F(SessionFsmUnitTest, SessionFsmInstantiationBasicSucceeds)
{
    EXPECT_NO_THROW({ SessionFsm session(events); }) << "Basic SessionFsm instantiation should not throw";
}

// Test 7: Test SessionFsm with engine factory instantiation
TEST_F(SessionFsmUnitTest, SessionFsmInstantiationWithFactorySucceeds)
{
    auto factory = std::make_shared<EngineFactory>(events);
    EXPECT_NO_THROW({ SessionFsm session(events, factory); })
        << "SessionFsm instantiation with factory should not throw";
}

// Test 8: Test copy/move semantics
TEST_F(SessionFsmUnitTest, SessionFsmCopyConstructorIsDeleted)
{
    // Verify that SessionFsm is not copyable (good for RAII)
    static_assert(!std::is_copy_constructible_v<SessionFsm>, "SessionFsm should not be copy constructible");
    static_assert(!std::is_copy_assignable_v<SessionFsm>, "SessionFsm should not be copy assignable");
}

// Test 9: Basic factory method functionality test
TEST_F(SessionFsmUnitTest, FactoryMethodsCreateComponentsReturnsValidPointers)
{
    TestableSessionFsm session(events);

    // Test strategy assigner creation
    auto assigner = session.testCreateStrategyAssigner(testTableProfile, 2);
    ASSERT_NE(assigner, nullptr) << "Strategy assigner should not be null";

    // Test player factory creation with the assigner
    auto playerFactory = session.testCreatePlayerFactory(events, assigner.get());
    ASSERT_NE(playerFactory, nullptr) << "Player factory should not be null";
}

// Test 10: Parameter validation tests
TEST_F(SessionFsmUnitTest, ValidateGameParametersWithValidDataDoesNotThrow)
{
    SessionFsmTestHelper session(events);
    GameData validGameData;
    validGameData.startMoney = 1000;

    StartData validStartData;
    validStartData.numberOfPlayers = 4;
    validStartData.startDealerPlayerId = 0;

    EXPECT_NO_THROW(session.validateGameParameters(validGameData, validStartData))
        << "Valid parameters should not throw";
}

TEST_F(SessionFsmUnitTest, ValidateGameParametersWithInvalidPlayerCountThrows)
{
    SessionFsmTestHelper session(events);
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

TEST_F(SessionFsmUnitTest, ValidateGameParametersWithInvalidMoneyThrows)
{
    SessionFsmTestHelper session(events);
    GameData invalidGameData;
    invalidGameData.startMoney = 0; // Invalid

    StartData startData;
    startData.numberOfPlayers = 4;
    startData.startDealerPlayerId = 0;

    EXPECT_THROW(session.validateGameParameters(invalidGameData, startData), std::invalid_argument)
        << "Zero start money should throw";
}

TEST_F(SessionFsmUnitTest, ValidateGameParametersWithInvalidDealerIdThrows)
{
    SessionFsmTestHelper session(events);
    GameData gameData;
    gameData.startMoney = 1000;

    StartData invalidStartData;
    invalidStartData.numberOfPlayers = 4;
    invalidStartData.startDealerPlayerId = 5; // Invalid - beyond player range

    EXPECT_THROW(session.validateGameParameters(gameData, invalidStartData), std::invalid_argument)
        << "Invalid dealer ID should throw";
}

// Test 11: Event firing tests
TEST_F(SessionFsmUnitTest, FireGameInitializedEventWithValidHandlerCallsEvent)
{
    bool eventCalled = false;
    int receivedSpeed = 0;

    GameEvents testEvents;
    testEvents.onGameInitialized = [&eventCalled, &receivedSpeed](int speed)
    {
        eventCalled = true;
        receivedSpeed = speed;
    };

    SessionFsmTestHelper session(testEvents);
    session.fireGameInitializedEvent(100);

    EXPECT_TRUE(eventCalled) << "Event should have been called";
    EXPECT_EQ(receivedSpeed, 100) << "Event should receive correct speed";
}

TEST_F(SessionFsmUnitTest, FireGameInitializedEventWithNullHandlerDoesNotCrash)
{
    GameEvents testEvents;
    testEvents.onGameInitialized = nullptr; // Null handler

    SessionFsmTestHelper session(testEvents);

    EXPECT_NO_THROW(session.fireGameInitializedEvent(50)) << "Null event handler should not crash";
}

// Test 12: Engine factory initialization tests
TEST_F(SessionFsmUnitTest, EnsureEngineFactoryInitializedWithoutFactoryCreatesFactory)
{
    SessionFsmTestHelper session(events);

    // Before initialization, createBoard should throw
    StartData testData{};
    testData.numberOfPlayers = 2;
    testData.startDealerPlayerId = 0;
    EXPECT_THROW(session.createBoard(testData), std::runtime_error) << "Should throw without factory";

    // After initialization, createBoard should work
    session.ensureEngineFactoryInitialized();
    EXPECT_NO_THROW(session.createBoard(testData)) << "Should not throw after factory initialization";
}

TEST_F(SessionFsmUnitTest, EnsureEngineFactoryInitializedWithExistingFactoryDoesNotReplace)
{
    auto factory = std::make_shared<EngineFactory>(events);
    TestableSessionFsm session(events, factory);
    EXPECT_TRUE(session.hasEngineFactory()) << "Should start with factory";

    // Test that it still works after calling the method
    SessionFsmTestHelper testSession(events, factory);
    testSession.ensureEngineFactoryInitialized();

    StartData testData{};
    testData.numberOfPlayers = 2;
    testData.startDealerPlayerId = 0;
    EXPECT_NO_THROW(testSession.createBoard(testData)) << "Should still work after calling ensure";
}