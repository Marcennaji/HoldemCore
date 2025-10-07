// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include <infra/ConsoleLogger.h>
#include <infra/eval/PsimHandEvaluationEngine.h>

#include "infra/persistence/SqliteDb.h"
#include "infra/persistence/SqlitePlayersStatisticsStore.h"

#include <ui/qtqml/QmlAppController.h>

#include <QString>
#include <QtCore>
#include <QtGui>
#include <QtQuick/QQuickWindow>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QDir>

using namespace std;
using namespace pkt::ui::qtqml;

std::string getDatabasePath()
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath); 
    return (appDataPath + "/HoldemCore.db").toStdString();
}

int main(int argc, char** argv)
{
    QGuiApplication app(argc, argv);
    
    // Set application metadata for QStandardPaths
    QCoreApplication::setApplicationName("HoldemCore");
    QCoreApplication::setOrganizationName("M87 Dev");
    QCoreApplication::setApplicationVersion("0.9");

    auto services = std::make_shared<pkt::core::AppServiceContainer>();
    
    services->setLogger(std::make_unique<pkt::infra::ConsoleLogger>());
    services->setHandEvaluationEngine(std::make_unique<pkt::infra::PsimHandEvaluationEngine>(services));
    
    auto db = std::make_unique<pkt::infra::SqliteDb>(getDatabasePath());
    services->setPlayersStatisticsStore(std::make_unique<pkt::infra::SqlitePlayersStatisticsStore>(std::move(db)));

    QmlAppController controller(services);
    
    if (!controller.initializeAndShow()) {
        return -1;
    }

    return app.exec();
}
