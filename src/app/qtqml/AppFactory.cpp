// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "AppFactory.h"
#include "../DependenciesFactory.h"

// UI controllers - need full definitions for QVariant::fromValue
#include <ui/qtqml/controller/AppController.h>
#include <ui/qtqml/controller/Bridge.h>
#include <ui/qtqml/controller/CardHelper.h>
#include <ui/qtqml/controller/TableViewModel.h>

// Qt QML
#include <QCoreApplication>
#include <QDir>
#include <QVariant>
#include <QtQml/QJSEngine>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlEngine>

namespace pkt::app::qtqml
{

std::unique_ptr<pkt::ui::qtqml::controller::AppController>
AppFactory::createApplication(LoggerType loggerType, HandEvaluatorType evaluatorType, DatabaseType dbType)
{
    auto logger = DependenciesFactory::createLogger(loggerType);
    auto handEvaluator = DependenciesFactory::createHandEvaluator(evaluatorType);
    auto statisticsStore = DependenciesFactory::createStatisticsStore(dbType);
    auto randomizer = DependenciesFactory::createRandomizer();

    return std::make_unique<pkt::ui::qtqml::controller::AppController>(logger, handEvaluator, statisticsStore,
                                                                       randomizer);
}

void AppFactory::registerQmlTypes()
{
    // Register CardHelper as a singleton type for QML
    // This allows QML to call CardHelper.getCardIndex("As") directly
    qmlRegisterSingletonType<pkt::ui::qtqml::controller::CardHelper>(
        "HoldemCore", 1, 0, "CardHelper",
        [](QQmlEngine*, QJSEngine*) -> QObject* { return new pkt::ui::qtqml::controller::CardHelper(); });
}

void AppFactory::configureQmlEngine(QQmlApplicationEngine& engine,
                                    pkt::ui::qtqml::controller::AppController* appController)
{
    QQmlContext* rootContext = engine.rootContext();

    // Register C++ objects with QML context
    rootContext->setContextProperty("appController", QVariant::fromValue(appController));
    rootContext->setContextProperty("tableViewModel", QVariant::fromValue(appController->getTableViewModel()));
    rootContext->setContextProperty("bridge", QVariant::fromValue(appController->getBridge()));

    // Expose card images path for QML (cards are copied to build directory by CMake)
    QString cardsPath = QCoreApplication::applicationDirPath() + "/cards";
    QString cleanPath = QDir::cleanPath(cardsPath);
    QString fileUrl = "file:///" + cleanPath.replace("\\", "/");
    rootContext->setContextProperty("cardsImagePath", fileUrl);
}

} // namespace pkt::app::qtqml
