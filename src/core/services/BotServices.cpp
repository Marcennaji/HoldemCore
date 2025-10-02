// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "BotServices.h"

namespace pkt::core
{

BotServices::BotServices(std::shared_ptr<ServiceContainer> baseContainer) : myBaseContainer(baseContainer)
{
}

Logger& BotServices::logger()
{
    return myBaseContainer->logger();
}

PlayersStatisticsStore& BotServices::playersStatisticsStore()
{
    return myBaseContainer->playersStatisticsStore();
}

HandEvaluationEngine& BotServices::handEvaluationEngine()
{
    return myBaseContainer->handEvaluationEngine();
}

Randomizer& BotServices::randomizer()
{
    return myBaseContainer->randomizer();
}

} // namespace pkt::core