// HoldemCore: A Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "ServiceContainer.h"
#include "DefaultRandomizer.h"
#include "core/interfaces/NullHandEvaluationEngine.h"
#include "core/interfaces/persistence/NullPlayersStatisticsStore.h"
#include "infra/ConsoleLogger.h"
#include "infra/eval/PsimHandEvaluationEngine.h"

namespace pkt::core
{

AppServiceContainer::AppServiceContainer()
        : myLogger(std::make_unique<pkt::infra::ConsoleLogger>()),
            myPlayersStatisticsStore(std::make_unique<NullPlayersStatisticsStore>()),
            myRandomizer(std::make_unique<DefaultRandomizer>()),
            myHandEvaluationEngine(std::make_unique<pkt::infra::PsimHandEvaluationEngine>())
{
}

ILogger& AppServiceContainer::logger()
{
    return *myLogger;
}

PlayersStatisticsStore& AppServiceContainer::playersStatisticsStore()
{
    return *myPlayersStatisticsStore;
}

HandEvaluationEngine& AppServiceContainer::handEvaluationEngine()
{
    return *myHandEvaluationEngine;
}

IRandomizer& AppServiceContainer::randomizer()
{
    return *myRandomizer;
}

} // namespace pkt::core