/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU Affero General Public License as            *
 * published by the Free Software Foundation, either version 3 of the        *
 * License, or (at your option) any later version.                           *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU Affero General Public License for more details.                       *
 *                                                                           *
 * You should have received a copy of the GNU Affero General Public License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#include <cstdlib>
#include <ctime>
#include <iostream>

#include "session.h"

#include <QString>
#include <QtCore>
#include <QtGui>
#include <QtWidgets/QApplication>

#include <Wincon.h>

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

#ifdef LOG_POKER_EXEC

#ifdef LOG_POKER_CONSOLE

    // output to console
    AllocConsole();
    freopen("conin$", "r", stdin);
    freopen("conout$", "w", stdout);
    freopen("conout$", "w", stderr);
    printf("Debugging Window:\n");

#else
    // output to log file
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

#endif

    QApplication app(argc, argv);
    int* p = NULL;

    AppDirectories dirs = AppDirectories::initialize();
    QString appPath = QString::fromStdString(dirs.appDataDir);
    QString logPath = QString::fromStdString(dirs.logDir);
    QString userPath = QString::fromStdString(dirs.userDataDir);

    auto logger = std::make_unique<ConsoleLogger>();
    GuiAppController controller(logger.get(), appPath, logPath, userPath);
    StartWindow* mainWindow = controller.createMainWindow();

    return app.exec();
}
