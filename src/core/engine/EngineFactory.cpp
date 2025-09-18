// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "EngineFactory.h"

#include <core/services/GlobalServices.h>
#include "core/engine/game/Board.h"
#include "strategy/LooseAggressiveBotStrategy.h"
#include "strategy/ManiacBotStrategy.h"
#include "strategy/TightAggressiveBotStrategy.h"

namespace pkt::core
{

EngineFactory::EngineFactory(const GameEvents& events) : myEvents(events)
{
}

EngineFactory::~EngineFactory() = default;

std::shared_ptr<Hand> EngineFactory::createHand(std::shared_ptr<EngineFactory> f, std::shared_ptr<IBoard> b,
                                                pkt::core::player::PlayerList seats,
                                                pkt::core::player::PlayerList actingPlayers, GameData gd, StartData sd)
{
    return std::make_shared<Hand>(myEvents, f, b, seats, actingPlayers, gd, sd);
}

std::shared_ptr<IBoard> EngineFactory::createBoard(unsigned dealerPosition)
{
    return std::make_shared<Board>(dealerPosition, myEvents);
}
} // namespace pkt::core
