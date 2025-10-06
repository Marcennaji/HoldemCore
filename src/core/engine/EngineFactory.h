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

#include <memory>
#include <vector>

#include "core/player/typedefs.h"

namespace pkt::core
{
class EngineFactory
{
  public:
    EngineFactory(const GameEvents&);
    
    // ISP-compliant constructor using focused service interfaces
    EngineFactory(const GameEvents& events,
                  std::shared_ptr<Logger> logger,
                  std::shared_ptr<HandEvaluationEngine> handEvaluator,
                  std::shared_ptr<PlayersStatisticsStore> statisticsStore = nullptr,
                  std::shared_ptr<ServiceContainer> serviceContainer = nullptr);    ~EngineFactory();

    virtual std::shared_ptr<Hand> createHand(std::shared_ptr<EngineFactory> f, std::shared_ptr<Board> b,
                                             pkt::core::player::PlayerList seats,
                                             pkt::core::player::PlayerList actingPlayers, GameData, StartData);

    virtual std::shared_ptr<Board> createBoard(unsigned dealerPosition);

  private:
    const GameEvents& m_events;
    mutable std::shared_ptr<pkt::core::ServiceContainer> m_services; // Legacy fallback
    
    // ISP-compliant focused dependencies
    std::shared_ptr<Logger> m_logger;
    std::shared_ptr<HandEvaluationEngine> m_handEvaluator;
    std::shared_ptr<PlayersStatisticsStore> m_statisticsStore;

    void ensureServicesInitialized() const;
    
    // ISP-compliant helper methods
    pkt::core::Logger& getLogger() const;
    pkt::core::HandEvaluationEngine& getHandEvaluationEngine() const;
    
  public:
    // Method to get service container for Hand (temporary during migration)
    std::shared_ptr<pkt::core::ServiceContainer> getServiceContainer() const;
};

} // namespace pkt::core
