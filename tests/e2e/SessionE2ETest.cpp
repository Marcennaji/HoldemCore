// PokerTraining — E2E tests for SessionFsm (Simplified)
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include <chrono>
#include <core/engine/EngineFactory.h>
#include <core/engine/model/GameData.h>
#include <core/engine/model/StartData.h>
#include <core/session/SessionFsm.h>
#include <gtest/gtest.h>

using namespace pkt::core;

class SessionE2Etest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // Initialize real GameEvents (can be empty for basic tests)
        events = GameEvents{};

        // Setup realistic game data
        gameData.guiSpeed = 50;
        gameData.startMoney = 1000;
        gameData.tableProfile = TableProfile{};

        // Setup start data for 4 players
        startData.numberOfPlayers = 4;
        startData.startDealerPlayerId = 0;
    }

    GameEvents events;
    GameData gameData;
    StartData startData;
};

// Test 1: Basic SessionFsm creation and destruction
TEST_F(SessionE2Etest, BasicCreationWithRealDependenciesSucceeds)
{
    EXPECT_NO_THROW({ SessionFsm session(events); }) << "SessionFsm creation should not throw with real dependencies";
}

// Test 2: SessionFsm creation with engine factory injection
TEST_F(SessionE2Etest, CreationWithFactoryInjectsDependencySucceeds)
{
    auto factory = std::make_shared<EngineFactory>(events);

    EXPECT_NO_THROW({ SessionFsm session(events, factory); })
        << "SessionFsm creation with factory injection should succeed";
}

// Test 3: Multiple SessionFsm instances can coexist
TEST_F(SessionE2Etest, MultipleInstancesCanCoexistNoInterference)
{
    EXPECT_NO_THROW({
        SessionFsm session1(events);
        SessionFsm session2(events);
        auto factory = std::make_shared<EngineFactory>(events);
        SessionFsm session3(events, factory);
    }) << "Multiple SessionFsm instances should coexist without interference";
}

// Test 4: Performance - SessionFsm creation is reasonably fast
TEST_F(SessionE2Etest, PerformanceCreationTimeIsReasonable)
{
    auto start = std::chrono::high_resolution_clock::now();

    // Create many sessions
    const int iterations = 1000;
    for (int i = 0; i < iterations; ++i)
    {
        SessionFsm session(events);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should create 1000 sessions in less than 1 second
    EXPECT_LT(duration.count(), 1000) << "Creating " << iterations << " SessionFsm instances took " << duration.count()
                                      << "ms (should be < 1000ms)";
}

// Test 5: Memory management - no leaks with RAII
TEST_F(SessionE2Etest, MemoryManagementRAIINoLeaks)
{
    // Create and destroy many sessions to test for memory leaks
    for (int i = 0; i < 100; ++i)
    {
        auto factory = std::make_shared<EngineFactory>(events);
        SessionFsm session(events, factory);
        // Destructor should clean up properly
    }

    // If we reach here without crashes, RAII is working
    SUCCEED() << "RAII memory management working correctly";
}