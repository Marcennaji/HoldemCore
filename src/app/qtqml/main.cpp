// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include <ui/qtqml/controller/AppController.h>
#include <ui/qtqml/controller/Bridge.h>
#include <ui/qtqml/controller/TableViewModel.h>
#include "AppFactory.h"

#include <QtCore>
#include <QtGui>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQuick/QQuickView>
#include <QtWidgets/QApplication>

using namespace std;
using namespace pkt::ui::qtqml;

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName("HoldemCore Mobile");
    QCoreApplication::setApplicationVersion("0.9");

    // Create application using the QML factory
    auto qmlController = pkt::app::qtqml::AppFactory::createApplication(
        pkt::app::LoggerType::Console, pkt::app::HandEvaluatorType::Psim, pkt::app::DatabaseType::Sqlite);

    // Create QML engine
    QQmlApplicationEngine engine;

    // Register C++ objects with QML context
    QQmlContext* rootContext = engine.rootContext();
    rootContext->setContextProperty("appController", qmlController.get());
    rootContext->setContextProperty("tableViewModel", qmlController->getTableViewModel());
    rootContext->setContextProperty("bridge", qmlController->getBridge());

    // Load the main QML file
    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject* obj, const QUrl& objUrl)
        {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.load(url);

    if (engine.rootObjects().isEmpty())
    {
        qCritical() << "Failed to load QML";
        return -1;
    }

    return app.exec();
}
