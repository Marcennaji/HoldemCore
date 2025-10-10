// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "AppFactory.h"

// Core interfaces
#include <core/interfaces/Logger.h>
#include <core/interfaces/HandEvaluationEngine.h>
#include <core/interfaces/persistence/PlayersStatisticsStore.h>
#include <core/interfaces/Randomizer.h>

// Infrastructure implementations
#include <infra/ConsoleLogger.h>
#include <infra/NullLogger.h>
#include <infra/eval/PsimHandEvaluationEngine.h>
#include <infra/persistence/SqliteDb.h>
#include <infra/persistence/SqlitePlayersStatisticsStore.h>
#include <infra/persistence/NullPlayersStatisticsStore.h>
#include <core/services/DefaultRandomizer.h>

// UI controller
#include <ui/qtwidgets/controller/GuiAppController.h>

// Qt includes for database path
#include <QStandardPaths>
#include <QDir>

namespace pkt::app {

std::unique_ptr<pkt::ui::qtwidgets::GuiAppController> 
AppFactory::createApplication(LoggerType loggerType, HandEvaluatorType evaluatorType, DatabaseType dbType) {
    // Create all dependencies
    auto logger = createLogger(loggerType);
    auto handEvaluator = createHandEvaluator(evaluatorType);
    auto statisticsStore = createStatisticsStore(dbType);
    auto randomizer = createRandomizer();

    // Create and return the main controller
    return std::make_unique<pkt::ui::qtwidgets::GuiAppController>(
        logger, handEvaluator, statisticsStore, randomizer);
}

std::unique_ptr<pkt::ui::qtwidgets::GuiAppController> 
AppFactory::createTestApp() {
    return createApplication(
        LoggerType::Null,
        HandEvaluatorType::Psim,  // Keep real evaluator for realistic tests
        DatabaseType::Sqlite  // Use real DB for persistent test data
    );
}

std::shared_ptr<pkt::core::Logger> 
AppFactory::createLogger(LoggerType type) {
    switch (type) {
        case LoggerType::Console: {
            auto logger = std::make_shared<pkt::infra::ConsoleLogger>();
            logger->setLogLevel(pkt::core::LogLevel::Info);
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

std::shared_ptr<pkt::core::HandEvaluationEngine> 
AppFactory::createHandEvaluator(HandEvaluatorType type) {
    switch (type) {
        case HandEvaluatorType::Psim:
        default:
            return std::make_shared<pkt::infra::PsimHandEvaluationEngine>();
    }
}

std::shared_ptr<pkt::core::PlayersStatisticsStore> 
AppFactory::createStatisticsStore(DatabaseType type) {
    switch (type) {
        case DatabaseType::Sqlite: {
            auto db = std::make_unique<pkt::infra::SqliteDb>(getDatabasePath());
            return std::make_shared<pkt::infra::SqlitePlayersStatisticsStore>(std::move(db));
        }
        
        case DatabaseType::Null:
        default:
            return std::make_shared<pkt::infra::NullPlayersStatisticsStore>();
    }
}

std::shared_ptr<pkt::core::Randomizer> 
AppFactory::createRandomizer() {
    return std::make_shared<pkt::core::DefaultRandomizer>();
}

std::string AppFactory::getDatabasePath() {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath); 
    return (appDataPath + "/HoldemCore.db").toStdString();
}

} // namespace pkt::app