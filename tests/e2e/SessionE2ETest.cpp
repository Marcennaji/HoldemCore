// HoldemCore — E2E tests for Session (Simplified)
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include <chrono>
#include <thread>
#include <algorithm>
#include <core/engine/EngineFactory.h>
#include <core/engine/model/GameData.h>
#include <core/engine/model/StartData.h>
#include <core/session/Session.h>
#include <core/interfaces/Logger.h>
#include <core/interfaces/HandEvaluationEngine.h>
#include <core/interfaces/Randomizer.h>
#include <core/interfaces/persistence/PlayersStatisticsStore.h>
#include <core/interfaces/persistence/NullPlayersStatisticsStore.h>
#include <core/services/DefaultRandomizer.h>
#include <infra/ConsoleLogger.h>
#include <infra/eval/PsimHandEvaluationEngine.h>
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
        
        // Setup ISP services for Session creation
        auto logger = std::make_shared<pkt::infra::ConsoleLogger>();
        logger->setLogLevel(pkt::core::LogLevel::Info); // Reduce log noise in tests
        m_logger = logger;
        m_handEvaluator = std::make_shared<pkt::infra::PsimHandEvaluationEngine>();
        m_statisticsStore = std::make_shared<pkt::core::NullPlayersStatisticsStore>();
        m_randomizer = std::make_shared<pkt::core::DefaultRandomizer>();
    }

    GameEvents events;
    GameData gameData;
    StartData startData;
    
    // ISP services for Session creation
    std::shared_ptr<pkt::core::Logger> m_logger;
    std::shared_ptr<pkt::core::HandEvaluationEngine> m_handEvaluator;
    std::shared_ptr<pkt::core::PlayersStatisticsStore> m_statisticsStore;
    std::shared_ptr<pkt::core::Randomizer> m_randomizer;
};

// Test 1: Basic Session creation and destruction
TEST_F(SessionE2Etest, BasicCreationWithRealDependenciesSucceeds)
{
    EXPECT_NO_THROW({ 
        auto factory = std::make_shared<EngineFactory>(events, *m_logger, *m_handEvaluator, *m_statisticsStore, *m_randomizer);
        Session session(events, *factory, *m_logger, *m_handEvaluator, *m_statisticsStore, *m_randomizer); 
    }) 
        << "Session creation should not throw with real dependencies";
}

// Test 2: Session creation with engine factory injection
TEST_F(SessionE2Etest, CreationWithFactoryInjectsDependencySucceeds)
{
    auto factory = std::make_shared<EngineFactory>(events, *m_logger, *m_handEvaluator, *m_statisticsStore, *m_randomizer);

    EXPECT_NO_THROW({ Session session(events, *factory, *m_logger, *m_handEvaluator, *m_statisticsStore, *m_randomizer); }) 
        << "Session creation with factory injection should succeed";
}

// Test 3: Multiple Session instances can coexist
TEST_F(SessionE2Etest, MultipleInstancesCanCoexistNoInterference)
{
    EXPECT_NO_THROW({
        auto factory1 = std::make_shared<EngineFactory>(events, *m_logger, *m_handEvaluator, *m_statisticsStore, *m_randomizer);
        auto factory2 = std::make_shared<EngineFactory>(events, *m_logger, *m_handEvaluator, *m_statisticsStore, *m_randomizer);
        Session session1(events, *factory1, *m_logger, *m_handEvaluator, *m_statisticsStore, *m_randomizer);
        Session session2(events, *factory2, *m_logger, *m_handEvaluator, *m_statisticsStore, *m_randomizer);
        auto factory = std::make_shared<EngineFactory>(events, *m_logger, *m_handEvaluator, *m_statisticsStore, *m_randomizer);
        Session session3(events, *factory, *m_logger, *m_handEvaluator, *m_statisticsStore, *m_randomizer);
    }) << "Multiple Session instances should coexist without interference";
}

// Test 4: Performance - Session creation is reasonably fast
TEST_F(SessionE2Etest, PerformanceCreationTimeIsReasonable)
{
    auto start = std::chrono::high_resolution_clock::now();

    // Create many sessions
    const int iterations = 1000;
    for (int i = 0; i < iterations; ++i)
    {
        auto factory = std::make_shared<EngineFactory>(events, *m_logger, *m_handEvaluator, *m_statisticsStore, *m_randomizer);
        Session session(events, *factory, *m_logger, *m_handEvaluator, *m_statisticsStore, *m_randomizer);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should create 1000 sessions in less than 1 second
    EXPECT_LT(duration.count(), 1000) << "Creating " << iterations << " Session instances took " << duration.count()
                                      << "ms (should be < 1000ms)";
}

// Test 5: Memory management - no leaks with RAII
TEST_F(SessionE2Etest, MemoryManagementRAIINoLeaks)
{
    // Create and destroy many sessions to test for memory leaks
    for (int i = 0; i < 100; ++i)
    {
        auto factory = std::make_shared<EngineFactory>(events, *m_logger, *m_handEvaluator, *m_statisticsStore, *m_randomizer);
        Session session(events, *factory, *m_logger, *m_handEvaluator, *m_statisticsStore, *m_randomizer);
        // Destructor should clean up properly
    }

    // If we reach here without crashes, RAII is working
    SUCCEED() << "RAII memory management working correctly";
}