// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "QmlAppController.h"
#include <QQmlContext>
#include <QUrl>
#include <QDebug>

namespace pkt {
namespace ui {
namespace qtqml {

QmlAppController::QmlAppController(std::shared_ptr<pkt::core::AppServiceContainer> services, QObject* parent)
    : QObject(parent)
    , m_services(std::move(services))
    , m_engine(std::make_unique<QQmlApplicationEngine>())
{
    // Register this controller with the QML context so it can be accessed from QML
    m_engine->rootContext()->setContextProperty("appController", this);
}

bool QmlAppController::initializeAndShow()
{
    // Load the main QML file
    const QUrl qmlFile(QStringLiteral("qrc:/qt/qml/qtqml/main.qml"));
    
    // Handle QML loading errors
    QObject::connect(m_engine.get(), &QQmlApplicationEngine::objectCreationFailed,
                     [](const QUrl &url) {
                         qWarning() << "Failed to load QML file:" << url;
                     });

    m_engine->load(qmlFile);
    
    // Check if the QML file was loaded successfully
    if (m_engine->rootObjects().isEmpty()) {
        qWarning() << "Failed to create root QML object";
        return false;
    }

    return true;
}

void QmlAppController::startNewGame()
{
    qDebug() << "Starting new game...";
    // TODO: Implement game start logic using m_services
}

void QmlAppController::loadGame()
{
    qDebug() << "Loading game...";
    // TODO: Implement load game logic using m_services
}

void QmlAppController::showStatistics()
{
    qDebug() << "Showing statistics...";
    // TODO: Implement statistics display using m_services
}

} // namespace qtqml
} // namespace ui
} // namespace pkt