// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include <infra/ConsoleLogger.h>
#include <infra/eval/PsimHandEvaluationEngine.h>
#include "core/services/ServiceContainer.h"
#include "infra/persistence/SqliteDb.h"
#include "infra/persistence/SqlitePlayersStatisticsStore.h"

#include <ui/qtwidgets/controller/GuiAppController.h>
#include <ui/qtwidgets/windows/StartWindow.h>

#include <QString>
#include <QtCore>
#include <QtGui>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QStandardPaths>
#include <QDir>

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;
using namespace pkt::ui::qtwidgets;


std::string getDatabasePath()
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath); 
    return (appDataPath + "/HoldemCore.db").toStdString();
}

int main(int argc, char** argv)
{
    // Initialize Qt resources
    Q_INIT_RESOURCE(cards);
    
    QApplication app(argc, argv);
    
    // Set application metadata for QStandardPaths
    QCoreApplication::setApplicationName("HoldemCore");
    QCoreApplication::setOrganizationName("PokerTraining");
    QCoreApplication::setApplicationVersion("1.0");

    auto services = std::make_shared<pkt::core::AppServiceContainer>();
    
    services->setLogger(std::make_unique<pkt::infra::ConsoleLogger>());
    services->setHandEvaluationEngine(std::make_unique<pkt::infra::PsimHandEvaluationEngine>(services));
    
    auto db = std::make_unique<pkt::infra::SqliteDb>(getDatabasePath());
    services->setPlayersStatisticsStore(std::make_unique<pkt::infra::SqlitePlayersStatisticsStore>(std::move(db)));

    // Display development warning
    QMessageBox::warning(nullptr, "Development Notice", 
                        "Welcome to HoldemCore!\n\n"
                        "The core poker engine is fully functional, but the Qt user interface "
                        "is currently under active development and may not be fully usable yet.");

    GuiAppController controller(services);
    auto* mainWindow = controller.createMainWindow();
    mainWindow->show();

    return app.exec();
}
