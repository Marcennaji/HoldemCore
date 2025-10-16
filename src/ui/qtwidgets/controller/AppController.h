// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

// AppController.h
#pragma once

#include <core/engine/GameEvents.h>
#include <core/ports/HandEvaluationEngine.h>
#include <core/ports/Logger.h>
#include <core/ports/PlayersStatisticsStore.h>
#include <core/ports/Randomizer.h>
#include <core/session/Session.h>

#include <QString>
#include <memory>

namespace pkt::core
{
class Session;
class EngineFactory;
} // namespace pkt::core

namespace pkt::ui::qtwidgets
{
class PokerTableWindow;
class StartWindow;
} // namespace pkt::ui::qtwidgets

namespace pkt::ui::qtwidgets::controller
{
class Bridge;
/**
 * @brief Main application controller for the Qt Widgets-based poker GUI.
 *
 * Manages the overall application flow, window creation, and coordination
 * between the poker engine core and the Qt-based user interface components.
 */
class AppController
{
  public:
    AppController(std::shared_ptr<pkt::core::Logger> logger,
                  std::shared_ptr<pkt::core::HandEvaluationEngine> handEvaluationEngine,
                  std::shared_ptr<pkt::core::PlayersStatisticsStore> statisticsStore,
                  std::shared_ptr<pkt::core::Randomizer> randomizer);
    ~AppController();

    pkt::ui::qtwidgets::StartWindow* createMainWindow();

  private:
    // Service dependencies (stored to ensure lifetime)
    std::shared_ptr<pkt::core::Logger> m_logger;
    std::shared_ptr<pkt::core::HandEvaluationEngine> m_handEvaluationEngine;
    std::shared_ptr<pkt::core::PlayersStatisticsStore> m_statisticsStore;
    std::shared_ptr<pkt::core::Randomizer> m_randomizer;

    // Application components
    std::shared_ptr<pkt::core::EngineFactory> m_engineFactory;
    std::unique_ptr<pkt::ui::qtwidgets::PokerTableWindow> m_pokerTableWindow;
    std::unique_ptr<pkt::core::Session> m_session;
    pkt::core::GameEvents m_events;
    std::unique_ptr<Bridge> m_bridge;
};
} // namespace pkt::ui::qtwidgets::controller
