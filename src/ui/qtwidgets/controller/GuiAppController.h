// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


// GuiAppController.h
#pragma once

#include <core/engine/GameEvents.h>
#include <core/session/Session.h>
#include <core/interfaces/Logger.h>
#include <core/interfaces/HandEvaluationEngine.h>
#include <core/interfaces/persistence/PlayersStatisticsStore.h>
#include <core/interfaces/Randomizer.h>

#include <QString>
#include <memory>

namespace pkt::core
{
class Session;
} // namespace pkt::core

namespace pkt::ui::qtwidgets
{
class PokerTableWindow;
class GuiBridgeWidgets;
class StartWindow;
/**
 * @brief Main application controller for the Qt Widgets-based poker GUI.
 * 
 * Manages the overall application flow, window creation, and coordination
 * between the poker engine core and the Qt-based user interface components.
 */
class GuiAppController
{
  public:
    GuiAppController(std::shared_ptr<pkt::core::Logger> logger,
                     std::shared_ptr<pkt::core::HandEvaluationEngine> handEvaluationEngine,
                     std::shared_ptr<pkt::core::PlayersStatisticsStore> statisticsStore,
                     std::shared_ptr<pkt::core::Randomizer> randomizer);
    ~GuiAppController();

    StartWindow* createMainWindow();

  private:
    std::unique_ptr<PokerTableWindow> m_pokerTableWindow;
    std::unique_ptr<pkt::core::Session> m_session;
    pkt::core::GameEvents m_events;
    std::unique_ptr<GuiBridgeWidgets> m_bridge;
};
} // namespace pkt::ui::qtwidgets
