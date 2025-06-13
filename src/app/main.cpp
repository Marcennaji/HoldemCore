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

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>

#include "core/session/Session.h"

#include <QString>
#include <QtCore>
#include <QtGui>
#include <QtWidgets/QApplication>

#include <infra/AppDirectories.h>
#include <infra/ConsoleLogger.h>
#include <ui/qtwidgets/controller/GuiAppController.h>
#include <ui/qtwidgets/startwindow/StartWindow.h>

#ifdef LOG_POKER_EXEC
#define _CRTDBG_MAP_ALLOC

#include <crtdbg.h>

#define ENABLE_LEAK_CHECK()                                                                                            \
    {                                                                                                                  \
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);                                                  \
    }
#endif

using namespace std;

class StartWindow;

int main(int argc, char** argv)
{

    // ENABLE_LEAK_CHECK();
    //_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
    //_CrtSetBreakAlloc(49937);

    /////// can be removed for non-qt-guis ////////////
    // QtSingleApplication a( argc, argv );

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

    char buff[20];
    struct tm* sTm;
    time_t now = time(0);
    sTm = gmtime(&now);
    strftime(buff, sizeof(buff), "%Y-%m-%d %Hh%M", sTm);
    string filename = dirs.logDir;
    filename += "/pokertraining_hands_";
    filename += buff;
    filename += ".log";
    std::ofstream out(filename);
    std::streambuf* coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(out.rdbuf());

#endif

    return app.exec();
}
