/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the MIT License                                     *


 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * MIT License for more details.                                             *
 *                                                                           *
 * You should have received a copy of the MIT License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/

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

    auto logger = std::make_unique<pkt::infra::ConsoleLogger>();
    GuiAppController controller(logger.get(), appPath, logPath, userPath);
    StartWindow* mainWindow = controller.createMainWindow();

#ifdef LOG_POKER_EXEC

    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm sTm = *std::gmtime(&now_c);

    std::ostringstream filenameStream;
    filenameStream << dirs.logDir << "/pokertraining_hands_" << std::put_time(&sTm, "%Y-%m-%d %Hh%M") << ".log";

    std::ofstream out(filenameStream.str());
    std::streambuf* coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(out.rdbuf());

#endif

    return app.exec();
}
