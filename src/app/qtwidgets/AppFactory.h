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

namespace pkt::ui::qtwidgets::controller
{
class AppController;
}

namespace pkt::app::qtwidgets
{

/**
 * @brief Factory for creating Qt Widgets application.
 *
 * This factory creates the Qt Widgets version of the application,
 * using DependenciesFactory for infrastructure dependencies.
 */
class AppFactory
{
  public:
    /**
     * @brief Creates a fully configured Qt Widgets application controller.
     *
     * @param loggerType Type of logger implementation to use
     * @param evaluatorType Type of hand evaluator implementation to use
     * @param dbType Type of database implementation to use
     * @return Unique pointer to configured AppController
     */
    static std::unique_ptr<pkt::ui::qtwidgets::controller::AppController>
    createApplication(LoggerType loggerType = LoggerType::Console,
                      HandEvaluatorType evaluatorType = HandEvaluatorType::Psim,
                      DatabaseType dbType = DatabaseType::Sqlite);

    /**
     * @brief Creates application optimized for testing.
     * @return Unique pointer to configured AppController with null implementations
     */
    static std::unique_ptr<pkt::ui::qtwidgets::controller::AppController> createTestApp();
};

} // namespace pkt::app::qtwidgets
