// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "AppFactory.h"
#include "../DependenciesFactory.h"

// UI controller
#include <ui/qtwidgets/controller/AppController.h>

namespace pkt::app::qtwidgets
{

std::unique_ptr<pkt::ui::qtwidgets::controller::AppController>
AppFactory::createApplication(LoggerType loggerType, HandEvaluatorType evaluatorType, DatabaseType dbType)
{
    auto logger = DependenciesFactory::createLogger(loggerType);
    auto handEvaluator = DependenciesFactory::createHandEvaluator(evaluatorType);
    auto statisticsStore = DependenciesFactory::createStatisticsStore(dbType);
    auto randomizer = DependenciesFactory::createRandomizer();

    return std::make_unique<pkt::ui::qtwidgets::controller::AppController>(logger, handEvaluator, statisticsStore,
                                                                      randomizer);
}

std::unique_ptr<pkt::ui::qtwidgets::controller::AppController> AppFactory::createTestApp()
{
    return createApplication(LoggerType::Null, HandEvaluatorType::Psim, DatabaseType::Sqlite);
}

} // namespace pkt::app::qtwidgets
