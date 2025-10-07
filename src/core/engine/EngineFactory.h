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
    
    EngineFactory(const GameEvents& events,
                  Logger& logger,
                  HandEvaluationEngine& handEvaluator,
                  PlayersStatisticsStore& statisticsStore,
                  Randomizer& randomizer);    
                  
    ~EngineFactory();

    virtual std::shared_ptr<Hand> createHand(std::shared_ptr<Board> b,
                                             pkt::core::player::PlayerList seats,
                                             pkt::core::player::PlayerList actingPlayers, GameData, StartData);

    virtual std::shared_ptr<Board> createBoard(unsigned dealerPosition);
    Logger& getLogger() const { return *m_logger; }
    HandEvaluationEngine& getHandEvaluationEngine() const { return *m_handEvaluator; }
    PlayersStatisticsStore& getPlayersStatisticsStore() const { return *m_statisticsStore; }
    Randomizer& getRandomizer() const { return *m_randomizer; }
    
  private:
    const GameEvents& m_events;
    Logger* m_logger;
    HandEvaluationEngine* m_handEvaluator;
    PlayersStatisticsStore* m_statisticsStore;
    Randomizer* m_randomizer;

};

} // namespace pkt::core
