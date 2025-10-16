// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "DependenciesFactory.h"

// Core interfaces
#include <core/ports/HandEvaluationEngine.h>
#include <core/ports/Logger.h>
#include <core/ports/PlayersStatisticsStore.h>
#include <core/ports/Randomizer.h>

// Infrastructure implementations
#include <adapters/infrastructure/hand_evaluation/PsimHandEvaluationEngine.h>
#include <adapters/infrastructure/logger/ConsoleLogger.h>
#include <adapters/infrastructure/logger/NullLogger.h>
#include <adapters/infrastructure/randomizer/DefaultRandomizer.h>
#include <adapters/infrastructure/statistics/NullPlayersStatisticsStore.h>
#include <adapters/infrastructure/statistics/sqlite/SqliteDb.h>
#include <adapters/infrastructure/statistics/sqlite/SqlitePlayersStatisticsStore.h>

// Qt includes for database path
#include <QDir>
#include <QStandardPaths>

namespace pkt::app
{

std::shared_ptr<pkt::core::Logger> DependenciesFactory::createLogger(LoggerType type)
{
    switch (type)
    {
    case LoggerType::Console:
    {
        auto logger = std::make_shared<pkt::infra::ConsoleLogger>();
        logger->setLogLevel(pkt::core::LogLevel::DecisionMaking);
        return logger;
    }

    case LoggerType::File:
        // TODO: Implement FileLogger when needed
        // For now, fall back to console logger
        return createLogger(LoggerType::Console);

    case LoggerType::Null:
    default:
        return std::make_shared<pkt::infra::NullLogger>();
    }
}

std::shared_ptr<pkt::core::HandEvaluationEngine> DependenciesFactory::createHandEvaluator(HandEvaluatorType type)
{
    switch (type)
    {
    case HandEvaluatorType::Psim:
    default:
        return std::make_shared<pkt::infra::PsimHandEvaluationEngine>();
    }
}

std::shared_ptr<pkt::core::PlayersStatisticsStore> DependenciesFactory::createStatisticsStore(DatabaseType type)
{
    switch (type)
    {
    case DatabaseType::Sqlite:
    {
        auto db = std::make_unique<pkt::infra::SqliteDb>(getDatabasePath());
        return std::make_shared<pkt::infra::SqlitePlayersStatisticsStore>(std::move(db));
    }

    case DatabaseType::Null:
    default:
        return std::make_shared<pkt::infra::NullPlayersStatisticsStore>();
    }
}

std::shared_ptr<pkt::core::Randomizer> DependenciesFactory::createRandomizer()
{
    return std::make_shared<pkt::core::DefaultRandomizer>();
}

std::string DependenciesFactory::getDatabasePath()
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath);
    return (appDataPath + "/HoldemCore.db").toStdString();
}

} // namespace pkt::app
