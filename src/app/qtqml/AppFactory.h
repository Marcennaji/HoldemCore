// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <memory>
#include "../DependenciesFactory.h"

// Forward declarations from core interfaces
namespace pkt::core
{
class Logger;
class HandEvaluationEngine;
class PlayersStatisticsStore;
class Randomizer;
} // namespace pkt::core

namespace pkt::ui::qtqml::controller
{
class AppController;
}

namespace pkt::app::qtqml
{

/**
 * @brief Factory for creating Qt Quick/QML application.
 *
 * This factory creates the Qt Quick/QML version of the application,
 * using DependenciesFactory for infrastructure dependencies.
 */
class AppFactory
{
  public:
    /**
     * @brief Creates a fully configured QML application controller.
     *
     * @param loggerType Type of logger implementation to use
     * @param evaluatorType Type of hand evaluator implementation to use
     * @param dbType Type of database implementation to use
     * @return Unique pointer to configured AppController
     */
    static std::unique_ptr<pkt::ui::qtqml::controller::AppController>
    createApplication(LoggerType loggerType = LoggerType::Console,
                      HandEvaluatorType evaluatorType = HandEvaluatorType::Psim,
                      DatabaseType dbType = DatabaseType::Sqlite);
};

} // namespace pkt::app::qtqml
