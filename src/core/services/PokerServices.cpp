// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "PokerServices.h"

namespace pkt::core
{

PokerServices::PokerServices(std::shared_ptr<ServiceContainer> baseContainer) : m_baseContainer(baseContainer)
{
}

Logger& PokerServices::logger()
{
    return m_baseContainer->logger();
}

PlayersStatisticsStore& PokerServices::playersStatisticsStore()
{
    return m_baseContainer->playersStatisticsStore();
}

HandEvaluationEngine& PokerServices::handEvaluationEngine()
{
    return m_baseContainer->handEvaluationEngine();
}

Randomizer& PokerServices::randomizer()
{
    return m_baseContainer->randomizer();
}

} // namespace pkt::core