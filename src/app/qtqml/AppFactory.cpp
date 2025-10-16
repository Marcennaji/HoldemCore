// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "AppFactory.h"
#include "../DependenciesFactory.h"

// UI controller
#include <ui/qtqml/controller/AppController.h>

namespace pkt::app::qtqml
{

std::unique_ptr<pkt::ui::qtqml::controller::AppController>
AppFactory::createApplication(LoggerType loggerType, HandEvaluatorType evaluatorType, DatabaseType dbType)
{
    auto logger = DependenciesFactory::createLogger(loggerType);
    auto handEvaluator = DependenciesFactory::createHandEvaluator(evaluatorType);
    auto statisticsStore = DependenciesFactory::createStatisticsStore(dbType);
    auto randomizer = DependenciesFactory::createRandomizer();

    return std::make_unique<pkt::ui::qtqml::controller::AppController>(logger, handEvaluator, statisticsStore, randomizer);
}

} // namespace pkt::app::qtqml
