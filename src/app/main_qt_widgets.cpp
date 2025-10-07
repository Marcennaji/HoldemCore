// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include <infra/ConsoleLogger.h>
#include <infra/eval/PsimHandEvaluationEngine.h>
#include <infra/persistence/SqliteDb.h>
#include <infra/persistence/SqlitePlayersStatisticsStore.h>
#include <core/services/DefaultRandomizer.h>
#include <core/interfaces/Logger.h>

#include <ui/qtwidgets/controller/GuiAppController.h>
#include <ui/qtwidgets/windows/StartWindow.h>
#include <ui/qtwidgets/theme/ThemeManager.h>

#include <QString>
#include <QtCore>
#include <QtGui>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QStandardPaths>
#include <QDir>

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
    Q_INIT_RESOURCE(cards);
    
    QApplication app(argc, argv);
    
    pkt::ui::qtwidgets::ThemeManager::applyLightTheme(app);
    
    QCoreApplication::setApplicationName("HoldemCore");
    QCoreApplication::setApplicationVersion("0.9");

    auto logger = std::make_shared<pkt::infra::ConsoleLogger>();
    logger->setLogLevel(pkt::core::LogLevel::Info);
    auto handEvaluator = std::make_shared<pkt::infra::PsimHandEvaluationEngine>();
    
    auto db = std::make_unique<pkt::infra::SqliteDb>(getDatabasePath());
    auto statisticsStore = std::make_shared<pkt::infra::SqlitePlayersStatisticsStore>(std::move(db));
    auto randomizer = std::make_shared<pkt::core::DefaultRandomizer>();

    GuiAppController controller(logger, handEvaluator, statisticsStore, randomizer);
    auto* mainWindow = controller.createMainWindow();
    mainWindow->show();

    return app.exec();
}
