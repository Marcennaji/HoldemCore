// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include <infra/AppDirectories.h>
#include <infra/ConsoleLogger.h>
#include <infra/eval/PsimHandEvaluationEngine.h>
#include "core/services/GlobalServices.h"
#include "infra/persistence/SqliteDb.h"
#include "infra/persistence/SqlitePlayersStatisticsStore.h"

#include <ui/qtwidgets/controller/GuiAppController.h>
#include <ui/qtwidgets/startwindow/StartWindow.h>

#include <QString>
#include <QtCore>
#include <QtGui>
#include <QtWidgets/QApplication>

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;
using namespace pkt::ui::qtwidgets;

int main(int argc, char** argv)
{
    pkt::infra::AppDirectories dirs = pkt::infra::AppDirectories::initialize();

    auto& services = pkt::core::GlobalServices::instance();
    services.setLogger(std::make_unique<pkt::infra::ConsoleLogger>());
    services.setHandEvaluationEngine(std::make_unique<pkt::infra::PsimHandEvaluationEngine>());
    auto db = std::make_unique<pkt::infra::SqliteDb>(dirs.logDir + string("/pokerTraining.db"));
    services.setPlayersStatisticsStore(std::make_unique<pkt::infra::SqlitePlayersStatisticsStore>(std::move(db)));

    GuiAppController controller(QString::fromStdString(dirs.appDataDir));
    controller.createMainWindow();

    return QApplication::exec();
}
