// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "EngineFactory.h"

#include "core/engine/hand/Hand.h"
#include "core/engine/model/GameData.h"
#include "core/engine/model/StartData.h"
#include "core/engine/game/Board.h"
#include "core/player/Player.h"
#include "core/services/ServiceContainer.h"
#include "core/interfaces/Logger.h"
#include "core/interfaces/HandEvaluationEngine.h"
#include "core/interfaces/persistence/PlayersStatisticsStore.h"
#include "core/interfaces/Randomizer.h"

#include <memory>
#include <vector>

#include "core/player/typedefs.h"

namespace pkt::core
{
class EngineFactory
{
  public:
    // Backward-compatible constructor for legacy code and tests
    explicit EngineFactory(const GameEvents& events);
    
    // ISP-compliant constructor with focused service interfaces (preferred)
    EngineFactory(const GameEvents& events,
                  std::shared_ptr<Logger> logger,
                  std::shared_ptr<HandEvaluationEngine> handEvaluator,
                  std::shared_ptr<PlayersStatisticsStore> statisticsStore,
                  std::shared_ptr<Randomizer> randomizer);    
                  
    ~EngineFactory();

    virtual std::shared_ptr<Hand> createHand(std::shared_ptr<EngineFactory> f, std::shared_ptr<Board> b,
                                             pkt::core::player::PlayerList seats,
                                             pkt::core::player::PlayerList actingPlayers, GameData, StartData);

    virtual std::shared_ptr<Board> createBoard(unsigned dealerPosition);

  private:
    const GameEvents& m_events;
    // ISP-compliant focused dependencies
    std::shared_ptr<Logger> m_logger;
    std::shared_ptr<HandEvaluationEngine> m_handEvaluator;
    std::shared_ptr<PlayersStatisticsStore> m_statisticsStore;
    std::shared_ptr<Randomizer> m_randomizer;
    
  public:
    // ISP-compliant accessors for focused services
    Logger& getLogger() const { return *m_logger; }
    HandEvaluationEngine& getHandEvaluationEngine() const { return *m_handEvaluator; }
    PlayersStatisticsStore& getPlayersStatisticsStore() const { return *m_statisticsStore; }
    Randomizer& getRandomizer() const { return *m_randomizer; }
    
  private:
    // Helper method to create ServiceContainer from focused services during migration
    std::shared_ptr<pkt::core::ServiceContainer> createServiceContainerFromFocusedServices() const;
    
  public:
    // Legacy method for backward compatibility (used by Hand.cpp)
    std::shared_ptr<pkt::core::ServiceContainer> getServiceContainer() const;
};

} // namespace pkt::core
