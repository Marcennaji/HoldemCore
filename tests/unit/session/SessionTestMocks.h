// HoldemCore — Unit tests for Session
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/engine/EngineFactory.h>
#include <core/interfaces/IBoard.h>
#include <core/session/Session.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace pkt::core::test
{

// Simplified Session for testing basic functionality
class TestableSession : public Session
{
  public:
    TestableSession(const GameEvents& events) : Session(events) {}
    TestableSession(const GameEvents& events, std::shared_ptr<EngineFactory> factory) : Session(events, factory) {}

    // Expose protected factory methods for testing (these are the core methods we still need to test)
    auto testCreateStrategyAssigner(const TableProfile& profile, int bots)
    {
        return createStrategyAssigner(profile, bots);
    }

    auto testCreatePlayerFactory(const GameEvents& events, player::StrategyAssigner* assigner)
    {
        return createPlayerFactory(events, assigner);
    }

    void testCreateBoard(const StartData& data)
    {
        createBoard(data); // May throw if no factory
    }

    // Simple way to test if engine factory was initialized (for dependency injection tests)
    bool hasEngineFactory() const
    {
        try
        {
            StartData testData{};
            testData.numberOfPlayers = 2;
            testData.startDealerPlayerId = 0;
            const_cast<TestableSession*>(this)->createBoard(testData);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
};

} // namespace pkt::core::test