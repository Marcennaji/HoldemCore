// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include <ui/qtqml/controller/AppController.h>
#include "AppFactory.h"

#include <QtCore>
#include <QtGui>
#include <QtQml/QQmlApplicationEngine>
#include <QtQuickControls2/QQuickStyle>
#include <QtWidgets/QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName("HoldemCore Mobile");
    QCoreApplication::setApplicationVersion("0.9");

    QQuickStyle::setStyle("Fusion");

    pkt::app::qtqml::AppFactory::registerQmlTypes();

    auto appController = pkt::app::qtqml::AppFactory::createApplication(
        pkt::app::LoggerType::Console, pkt::app::HandEvaluatorType::Psim, pkt::app::DatabaseType::Sqlite);

    QQmlApplicationEngine engine;
    pkt::app::qtqml::AppFactory::configureQmlEngine(engine, appController.get());

    const QUrl url(QStringLiteral("qrc:/HoldemCore/qml/Main.qml"));

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
