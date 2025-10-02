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
        : m_logger(std::make_unique<pkt::infra::ConsoleLogger>()),
            m_playersStatisticsStore(std::make_unique<NullPlayersStatisticsStore>()),
            m_randomizer(std::make_unique<DefaultRandomizer>()),
            m_handEvaluationEngine(std::make_unique<pkt::infra::PsimHandEvaluationEngine>())
{
}

Logger& AppServiceContainer::logger()
{
    return *m_logger;
}

PlayersStatisticsStore& AppServiceContainer::playersStatisticsStore()
{
    return *m_playersStatisticsStore;
}

HandEvaluationEngine& AppServiceContainer::handEvaluationEngine()
{
    return *m_handEvaluationEngine;
}

Randomizer& AppServiceContainer::randomizer()
{
    return *m_randomizer;
}

} // namespace pkt::core