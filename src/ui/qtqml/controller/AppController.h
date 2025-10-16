// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/engine/GameEvents.h>
#include <core/ports/HandEvaluationEngine.h>
#include <core/ports/Logger.h>
#include <core/ports/PlayersStatisticsStore.h>
#include <core/ports/Randomizer.h>
#include <core/session/Session.h>

#include <QObject>
#include <memory>

namespace pkt::core
{
class Session;
class EngineFactory;
} // namespace pkt::core

namespace pkt::ui::qtqml::controller
{
class TableViewModel;
class Bridge;

/**
 * @brief Main application controller for the Qt Quick/QML-based poker GUI.
 *
 * Manages the overall application flow for the QML mobile interface,
 * coordinating between the poker engine core and the QML-based user interface.
 *
 * This controller follows the same pattern as AppController but is designed
 * specifically for Qt Quick/QML with proper C++ to QML data binding via view models.
 */
class AppController : public QObject
{
    Q_OBJECT

  public:
    AppController(std::shared_ptr<pkt::core::Logger> logger,
                  std::shared_ptr<pkt::core::HandEvaluationEngine> handEvaluationEngine,
                  std::shared_ptr<pkt::core::PlayersStatisticsStore> statisticsStore,
                  std::shared_ptr<pkt::core::Randomizer> randomizer, QObject* parent = nullptr);
    ~AppController();

    // Access to components for QML context registration
    TableViewModel* getTableViewModel() const { return m_tableViewModel.get(); }
    pkt::core::Session* getSession() const { return m_session.get(); }
    Bridge* getBridge() const { return m_bridge.get(); }

  public slots:
    // Called from QML SetupScreen when user starts a game
    void startGame(int playerCount, int smallBlind, int bigBlind, const QString& botProfile, double gameSpeed);

    // Called from QML TableScreen when user exits to setup
    void exitToSetup();

  private:
    // Service dependencies (stored to ensure lifetime)
    std::shared_ptr<pkt::core::Logger> m_logger;
    std::shared_ptr<pkt::core::HandEvaluationEngine> m_handEvaluationEngine;
    std::shared_ptr<pkt::core::PlayersStatisticsStore> m_statisticsStore;
    std::shared_ptr<pkt::core::Randomizer> m_randomizer;

    // Application components
    std::shared_ptr<pkt::core::EngineFactory> m_engineFactory;
    std::unique_ptr<pkt::core::Session> m_session;
    pkt::core::GameEvents m_events;
    std::unique_ptr<Bridge> m_bridge;
    std::unique_ptr<TableViewModel> m_tableViewModel;
};
} // namespace pkt::ui::qtqml::controller
