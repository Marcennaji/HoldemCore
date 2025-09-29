// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/services/ServiceContainer.h>
#include <QtQml/QQmlApplicationEngine>
#include <QObject>
#include <memory>

namespace pkt {
namespace ui {
namespace qtqml {

/**
 * @brief Controller class for the QML application
 * 
 * This class manages the QML application lifecycle and bridges between
 * the core services and the QML user interface.
 */
class QmlAppController : public QObject
{
    Q_OBJECT

public:
    explicit QmlAppController(std::shared_ptr<pkt::core::AppServiceContainer> services, QObject* parent = nullptr);
    ~QmlAppController() = default;

    /**
     * @brief Initialize and show the main QML window
     * @return true if initialization was successful, false otherwise
     */
    bool initializeAndShow();

public slots:
    /**
     * @brief Handle starting a new game
     */
    void startNewGame();

    /**
     * @brief Handle loading an existing game
     */
    void loadGame();

    /**
     * @brief Handle showing statistics
     */
    void showStatistics();

private:
    std::shared_ptr<pkt::core::AppServiceContainer> m_services;
    std::unique_ptr<QQmlApplicationEngine> m_engine;
};

} // namespace qtqml
} // namespace ui
} // namespace pkt