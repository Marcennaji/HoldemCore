// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include <ui/qtqml/controller/AppController.h>
#include <ui/qtqml/controller/Bridge.h>
#include <ui/qtqml/controller/TableViewModel.h>
#include "AppFactory.h"

#include <QDir>
#include <QtCore>
#include <QtGui>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQuick/QQuickView>
#include <QtQuickControls2/QQuickStyle>
#include <QtWidgets/QApplication>

using namespace std;
using namespace pkt::ui::qtqml;

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName("HoldemCore Mobile");
    QCoreApplication::setApplicationVersion("0.9");

    // Set Qt Quick style to Fusion to support custom styling
    QQuickStyle::setStyle("Fusion");

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

    // Expose card images path for QML (cards are copied to build directory by CMake)
    QString cardsPath = QCoreApplication::applicationDirPath() + "/cards";
    QString cleanPath = QDir::cleanPath(cardsPath);
    QString fileUrl = "file:///" + cleanPath.replace("\\", "/");
    rootContext->setContextProperty("cardsImagePath", fileUrl);

    // Load the main QML file
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
