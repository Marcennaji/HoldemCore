// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include <infra/AppDirectories.h>
#include <infra/ConsoleLogger.h>
#include <ui/qtwidgets/controller/GuiAppController.h>
#include <ui/qtwidgets/startwindow/StartWindow.h>
#include "core/session/Session.h"

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

class StartWindow;

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    int* p = NULL;
    pkt::infra::AppDirectories dirs = pkt::infra::AppDirectories::initialize();

    QString appPath = QString::fromStdString(dirs.appDataDir);
    QString logPath = QString::fromStdString(dirs.logDir);
    QString userPath = QString::fromStdString(dirs.userDataDir);

    // auto logger = std::make_shared<pkt::infra::ConsoleLogger>();
    GuiAppController controller(appPath, logPath, userPath);
    StartWindow* mainWindow = controller.createMainWindow();

    return app.exec();
}
