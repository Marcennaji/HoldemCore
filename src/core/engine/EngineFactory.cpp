// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "EngineFactory.h"
#include "core/engine/game/Board.h"
#include "strategy/LooseAggressiveBotStrategy.h"
#include "strategy/ManiacBotStrategy.h"
#include "strategy/TightAggressiveBotStrategy.h"
#include "core/engine/EngineDefs.h"

#include <stdexcept>

namespace pkt::core
{

EngineFactory::EngineFactory(const GameEvents& events, 
                             Logger& logger,
                             HandEvaluationEngine& handEvaluator,
                             PlayersStatisticsStore& statisticsStore,
                             Randomizer& randomizer)
    : m_events(events), m_logger(&logger), m_handEvaluator(&handEvaluator), m_statisticsStore(&statisticsStore), m_randomizer(&randomizer)
{
}

EngineFactory::~EngineFactory() = default;


std::shared_ptr<Hand> EngineFactory::createHand(std::shared_ptr<Board> b,
                                                pkt::core::player::PlayerList seats,
                                                pkt::core::player::PlayerList actingPlayers, GameData gd, StartData sd)
{
    return std::make_shared<Hand>(m_events, b, seats, actingPlayers, gd, sd, *m_logger, *m_statisticsStore, *m_randomizer, *m_handEvaluator);
}

std::shared_ptr<Board> EngineFactory::createBoard(unsigned dealerPosition)
{
    return std::make_shared<Board>(dealerPosition, m_events, *m_logger, *m_handEvaluator);
}


} // namespace pkt::core
