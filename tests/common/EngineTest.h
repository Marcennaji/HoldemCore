// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "common/common.h"

#include "adapters/infrastructure/logger/NullLogger.h"
#include "adapters/infrastructure/statistics/NullPlayersStatisticsStore.h"
#include "core/engine/EngineFactory.h"
#include "core/engine/game/Board.h"
#include "core/engine/model/GameData.h"
#include "core/engine/model/GameState.h"
#include "core/player/Player.h"
#include "core/player/typedefs.h"
#include "core/ports/HandEvaluationEngine.h"
#include "core/ports/Logger.h"
#include "core/ports/PlayersStatisticsStore.h"
#include "core/ports/Randomizer.h"

#include <list>
#include <memory>

namespace pkt::test
{

class EngineTest : public ::testing::Test
{
  protected:
    void SetUp() override;
    void TearDown() override;

    void initializeHandWithPlayers(size_t activePlayerCount, pkt::core::GameData);
    void setupHandWithoutInitialize(size_t activePlayerCount, pkt::core::GameData gameData);
    void checkPostRiverConditions();
    bool isPlayerStillActive(unsigned id) const;

    pkt::core::Logger& getLogger() const;
    std::shared_ptr<pkt::core::Logger> getLoggerService() const;
    std::shared_ptr<pkt::core::HandEvaluationEngine> getHandEvaluationEngineService() const;
    std::shared_ptr<pkt::core::PlayersStatisticsStore> getPlayersStatisticsStoreService() const;
    std::shared_ptr<pkt::core::Randomizer> getRandomizerService() const;

    virtual pkt::core::LogLevel getTestLogLevel() const { return pkt::core::LogLevel::Quiet; }

    pkt::core::GameEvents m_events;
    std::shared_ptr<pkt::core::EngineFactory> m_factory;
    std::shared_ptr<pkt::core::Hand> m_hand;
    std::shared_ptr<pkt::core::Board> m_board;

    std::shared_ptr<std::list<std::shared_ptr<pkt::core::player::Player>>> m_seatsList;
    std::shared_ptr<std::list<std::shared_ptr<pkt::core::player::Player>>> m_actingPlayersList;
    pkt::core::GameState m_lastGameState = pkt::core::None;

    pkt::core::GameData gameData;

  protected:
    std::shared_ptr<pkt::core::Logger> m_logger;
    std::shared_ptr<pkt::core::HandEvaluationEngine> m_handEvaluationEngine;
    std::shared_ptr<pkt::core::PlayersStatisticsStore> m_playersStatisticsStore;
    std::shared_ptr<pkt::core::Randomizer> m_randomizer;

  private:
    void createPlayersLists(size_t playerCount);
    const int startDealerPlayerId{0}; // Starting dealer player ID, can be set to any valid player ID
};
} // namespace pkt::test