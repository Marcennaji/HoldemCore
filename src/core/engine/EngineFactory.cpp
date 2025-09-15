// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "EngineFactory.h"

#include <core/services/GlobalServices.h>
#include "core/engine/BoardFsm.h"
#include "strategy/LooseAggressiveBotStrategy.h"
#include "strategy/ManiacBotStrategy.h"
#include "strategy/TightAggressiveBotStrategy.h"

namespace pkt::core
{

EngineFactory::EngineFactory(const GameEvents& events) : myEvents(events)
{
}

EngineFactory::~EngineFactory() = default;

std::shared_ptr<HandFsm> EngineFactory::createHandFsm(std::shared_ptr<EngineFactory> f, std::shared_ptr<IBoard> b,
                                                      pkt::core::player::PlayerFsmList seats,
                                                      pkt::core::player::PlayerFsmList actingPlayers, GameData gd,
                                                      StartData sd)
{
    return std::shared_ptr<HandFsm>(new HandFsm(myEvents, f, b, seats, actingPlayers, gd, sd));
}

std::shared_ptr<IBoard> EngineFactory::createBoardFsm(unsigned dealerPosition)
{
    return std::shared_ptr<IBoard>(new BoardFsm(dealerPosition, myEvents));
}
} // namespace pkt::core
