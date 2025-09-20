// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "PokerServices.h"

namespace pkt::core
{

PokerServices::PokerServices(std::shared_ptr<ServiceContainer> baseContainer) : myBaseContainer(baseContainer)
{
}

ILogger& PokerServices::logger()
{
    return myBaseContainer->logger();
}

PlayersStatisticsStore& PokerServices::playersStatisticsStore()
{
    return myBaseContainer->playersStatisticsStore();
}

HandEvaluationEngine& PokerServices::handEvaluationEngine()
{
    return myBaseContainer->handEvaluationEngine();
}

IRandomizer& PokerServices::randomizer()
{
    return myBaseContainer->randomizer();
}

} // namespace pkt::core