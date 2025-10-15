// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "AppFactory.h"

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

// UI controller
#include <ui/qtwidgets/controller/GuiAppController.h>

// Qt includes for database path
#include <QDir>
#include <QStandardPaths>

namespace pkt::app
{

std::unique_ptr<pkt::ui::qtwidgets::GuiAppController>
AppFactory::createApplication(LoggerType loggerType, HandEvaluatorType evaluatorType, DatabaseType dbType)
{

    auto logger = createLogger(loggerType);
    auto handEvaluator = createHandEvaluator(evaluatorType);
    auto statisticsStore = createStatisticsStore(dbType);
    auto randomizer = createRandomizer();

    return std::make_unique<pkt::ui::qtwidgets::GuiAppController>(logger, handEvaluator, statisticsStore, randomizer);
}

std::unique_ptr<pkt::ui::qtwidgets::GuiAppController> AppFactory::createTestApp()
{
    return createApplication(LoggerType::Null, HandEvaluatorType::Psim, DatabaseType::Sqlite);
}

std::shared_ptr<pkt::core::Logger> AppFactory::createLogger(LoggerType type)
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

std::shared_ptr<pkt::core::HandEvaluationEngine> AppFactory::createHandEvaluator(HandEvaluatorType type)
{
    switch (type)
    {
    case HandEvaluatorType::Psim:
    default:
        return std::make_shared<pkt::infra::PsimHandEvaluationEngine>();
    }
}

std::shared_ptr<pkt::core::PlayersStatisticsStore> AppFactory::createStatisticsStore(DatabaseType type)
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

std::shared_ptr<pkt::core::Randomizer> AppFactory::createRandomizer()
{
    return std::make_shared<pkt::core::DefaultRandomizer>();
}

std::string AppFactory::getDatabasePath()
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath);
    return (appDataPath + "/HoldemCore.db").toStdString();
}

} // namespace pkt::app