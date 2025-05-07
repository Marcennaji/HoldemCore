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
#include "startwindowimpl.h"
#include <engine/Player.h>
#include <gamedata.h>
#include <engine/tools.h>
#include <engine/game.h>
#include <qt/guiwrapper.h>
#include <configfile.h>
#include <qt/gametable/gametableimpl.h>
#include <qt/newgamedialog/newgamedialogimpl.h>
#include <qt/mymessagedialog/mymessagedialogimpl.h>
#include <qt/gametable/log/guilog.h>

using namespace std;

startWindowImpl::startWindowImpl(ConfigFile *c, Log *l)
	: myConfig(c), myLog(l)
{

	myGuiInterface.reset(new GuiWrapper(myConfig, this));

	mySession.reset(new Session(myGuiInterface.get(), myConfig, myLog));
	mySession->init(); // TODO handle error
	myLog->init();
	// myGuiInterface->setSession(session);

	myGuiInterface->getGuiLog()->setSqliteLogFileName(myLog->getSqliteLogFileName());

	setupUi(this);
	this->setWindowTitle(QString(tr("PokerTraining %1").arg(POKERTRAINING_BETA_RELEASE_STRING)));
	this->installEventFilter(this);

	//Widgets Grafiken per Stylesheets setzen
	QString myAppDataPath = QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str());
	this->setWindowIcon(QIcon(myAppDataPath+"gfx/gui/misc/windowicon.png"));

	this->setStatusBar(0);

	// 	Dialogs
	myNewGameDialog = new newGameDialogImpl(this, myConfig);

	connect( actionStartGame, SIGNAL( triggered() ), this, SLOT( callNewGameDialog() ) );
	connect( pushButtonStartGame, SIGNAL( clicked() ), this, SLOT( callNewGameDialog() ) );

	this->show();

}

startWindowImpl::~startWindowImpl()
{
}

void startWindowImpl::callNewGameDialog()
{
	if(myConfig->readConfigInt("ShowGameSettingsDialogOnNewGame")) {

		myNewGameDialog->exec();
		if (myNewGameDialog->result() == QDialog::Accepted ) {
			startNewGame(myNewGameDialog);
		}
	}
	else {
		startNewGame();
	}
}

void startWindowImpl::startNewGame(newGameDialogImpl *v)
{

	this->hide();
	myGuiInterface->getW()->show();

	//get values from  game dialog
	GameData gameData;
	if(v) {
		// Set Game Data
		gameData.maxNumberOfPlayers = v->spinBox_quantityPlayers->value();
		gameData.startMoney = v->spinBox_startCash->value();
		gameData.firstSmallBlind = 12;

		//Speeds
		gameData.guiSpeed = 8;

		if (v->radioButton_opponentsLooseAggressive->isChecked())
			gameData.tableProfile = LARGE_AGRESSIVE_OPPONENTS;
		else
		if (v->radioButton_opponentsTightAgressive->isChecked())
			gameData.tableProfile = TIGHT_AGRESSIVE_OPPONENTS;
		else
			gameData.tableProfile = RANDOM_OPPONENTS;

	}
	// start with default values
	else {
		// Set Game Data
		gameData.maxNumberOfPlayers = myConfig->readConfigInt("NumberOfPlayers");
		gameData.startMoney = myConfig->readConfigInt("StartCash");
		gameData.firstSmallBlind =  myConfig->readConfigInt("FirstSmallBlind");
		gameData.tableProfile = TIGHT_AGRESSIVE_OPPONENTS;

		//Speeds
		gameData.guiSpeed = myConfig->readConfigInt("GameSpeed");
			
	}
	// Set dealer pos.
	StartData startData;
	int tmpDealerPos = 0;
	startData.numberOfPlayers = gameData.maxNumberOfPlayers;

	Tools::GetRand(0, startData.numberOfPlayers-1, 1, &tmpDealerPos);
	//if(DEBUG_MODE) {
	//    tmpDealerPos = 4;
	//}
	startData.startDealerPlayerId = static_cast<unsigned>(tmpDealerPos);

	//some gui modifications
	myGuiInterface->getW()->GameModification();

	//Start Game!!!
	mySession->startGame(gameData, startData);
}


bool startWindowImpl::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::Close) {
		event->ignore();
		//        mySession->getLog()->closeLogDbAtExit();
		return QMainWindow::eventFilter(obj, event);
	} else {
		// pass the event on to the parent class
		return QMainWindow::eventFilter(obj, event);
	}
}
