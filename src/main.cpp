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

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <QtWidgets/QApplication>

#include <QtGui>
#include <QtCore>

#include <Wincon.h>

#include <session.h>
#include <qt/startwindow/startwindowimpl.h>
#include <configfile.h>
#include <engine/log.h>
#include <game_defs.h>

#ifdef LOG_POKER_EXEC
#define _CRTDBG_MAP_ALLOC

#include <crtdbg.h>

#define ENABLE_LEAK_CHECK() \
			{ \
				_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); \
			}
#endif

using namespace std;

class startWindowImpl;
class Game;

int main( int argc, char **argv )
{

	//ENABLE_LEAK_CHECK();
	//_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
	//_CrtSetBreakAlloc(49937);

	/////// can be removed for non-qt-guis ////////////
	//QtSingleApplication a( argc, argv );

	QApplication a( argc, argv );

	//create defaultconfig
	ConfigFile *myConfig = new ConfigFile(argv[0], false);
	Log *myLog = new Log(myConfig);


#ifdef LOG_POKER_EXEC

#ifdef LOG_POKER_CONSOLE

	// output to console
	AllocConsole();
	freopen("conin$","r",stdin);
	freopen("conout$","w",stdout);
	freopen("conout$","w",stderr);
	printf("Debugging Window:\n");

#else
	// output to log file
	char buff[20];
	struct tm *sTm;
	time_t now = time (0);
	sTm = gmtime (&now);
	strftime (buff, sizeof(buff), "%Y-%m-%d %Hh%M", sTm);
	string filename = myConfig->readConfigString("LogDir");
	filename += "/pokertraining_hands_";
	filename += buff;
	filename += ".log";
	std::ofstream out(filename);
	std::streambuf *coutbuf = std::cout.rdbuf(); 
	std::cout.rdbuf(out.rdbuf()); 

#endif

#endif
 
    QString	myAppDataPath = QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str());
 
	QFontDatabase::addApplicationFont (myAppDataPath +"fonts/n019003l.pfb");
	QFontDatabase::addApplicationFont (myAppDataPath +"fonts/VeraBd.ttf");
	QFontDatabase::addApplicationFont (myAppDataPath +"fonts/c059013l.pfb");
	QFontDatabase::addApplicationFont (myAppDataPath +"fonts/DejaVuSans-Bold.ttf");

	QString font1String("QApplication, QWidget, QDialog { font-size: 12px; }");
	a.setStyleSheet(font1String + " QDialogButtonBox, QMessageBox { dialogbuttonbox-buttons-have-icons: 1; dialog-ok-icon: url(:/gfx/dialog_ok_apply.png); dialog-cancel-icon: url(:/gfx/dialog_close.png); dialog-close-icon: url(:/gfx/dialog_close.png); dialog-yes-icon: url(:/gfx/dialog_ok_apply.png); dialog-no-icon: url(:/gfx/dialog_close.png) }");

 	qRegisterMetaType<unsigned>("unsigned");
	qRegisterMetaType<std::shared_ptr<Game> >("std::shared_ptr<Game>");

	startWindowImpl mainWin(myConfig,myLog);

	int retVal = a.exec();
	
	//_CrtCheckMemory();

	//_CrtDumpMemoryLeaks();

	return retVal;

}
