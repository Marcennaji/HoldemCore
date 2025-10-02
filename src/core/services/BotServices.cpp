// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "BotServices.h"

namespace pkt::core
{

BotServices::BotServices(std::shared_ptr<ServiceContainer> baseContainer) : m_baseContainer(baseContainer)
{
}

Logger& BotServices::logger()
{
    return m_baseContainer->logger();
}

PlayersStatisticsStore& BotServices::playersStatisticsStore()
{
    return m_baseContainer->playersStatisticsStore();
}

HandEvaluationEngine& BotServices::handEvaluationEngine()
{
    return m_baseContainer->handEvaluationEngine();
}

Randomizer& BotServices::randomizer()
{
    return m_baseContainer->randomizer();
}

} // namespace pkt::core