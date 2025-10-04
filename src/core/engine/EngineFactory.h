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
#include "core/services/PokerServices.h"
#include "core/interfaces/services/LoggerProvider.h"
#include "core/interfaces/services/HandEvaluationProvider.h"

#include <memory>
#include <vector>

#include "core/player/typedefs.h"

namespace pkt::core
{
class EngineFactory
{
  public:
    EngineFactory(const GameEvents&);

    // Constructor with PokerServices for dependency injection
    EngineFactory(const GameEvents&, std::shared_ptr<PokerServices> services);
    
    // Constructor with focused dependencies (ISP-compliant)
    EngineFactory(const GameEvents& events, 
                  std::shared_ptr<LoggerProvider> loggerProvider,
                  std::shared_ptr<HandEvaluationProvider> handEvalProvider);

    ~EngineFactory();

    virtual std::shared_ptr<Hand> createHand(std::shared_ptr<EngineFactory> f, std::shared_ptr<Board> b,
                                             pkt::core::player::PlayerList seats,
                                             pkt::core::player::PlayerList actingPlayers, GameData, StartData);

    virtual std::shared_ptr<Board> createBoard(unsigned dealerPosition);

  private:
    const GameEvents& m_events;
    std::shared_ptr<PokerServices> m_services; // Injected service container (legacy)
    
    // Focused dependencies (ISP-compliant)
    std::shared_ptr<LoggerProvider> m_loggerProvider;
    std::shared_ptr<HandEvaluationProvider> m_handEvalProvider;

    void ensureServicesInitialized();
    
    // Helper methods to get services through focused interfaces or legacy container
    Logger& getLogger();
    HandEvaluationEngine& getHandEvaluationEngine();
};

} // namespace pkt::core
