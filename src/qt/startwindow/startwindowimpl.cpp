/*****************************************************************************
 * PokerTraining - THNL training software, based on the PokerTH GUI          *
 * Copyright (C) 2013 Marc Ennaji                                            *
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
#include <qt/aboutPokerTraining/aboutPokerTrainingimpl.h>
#include <qt/mymessagedialog/mymessagedialogimpl.h>
#include <qt/mymessagebox/mymessagebox.h>
#include <qt/settingsdialog/settingsdialogimpl.h>
#include <qt/settingsdialog/selectavatardialog/selectavatardialogimpl.h>
#include <qt/changecompleteblindsdialog/changecompleteblindsdialogimpl.h>
#include <qt/gametable/log/guilog.h>

using namespace std;

startWindowImpl::startWindowImpl(ConfigFile *c, Log *l)
	: myConfig(c), myLog(l), msgBoxOutdatedVersionActive(false)
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

#ifdef GUI_800x480
#ifdef ANDROID
    this->setMenuBar(0);
    //	this->menubar->clear();
    //	this->menubar->hide();

    //check if custom background picture for the resolution is there. Otherwise create it!
    QString UserDataDir = QString::fromUtf8(myConfig->readConfigString("UserDataDir").c_str());
    QScreen* dw = QGuiApplication::primaryScreen();
    int screenWidth = dw->geometry().width();
    int screenHeight = dw->geometry().height();
    QString customStartWindowBgFileString(UserDataDir+"/startwindowbg10_"+QString::number(screenWidth)+"x"+QString::number(screenHeight)+".png");
    QFile customStartWindowBgFile(customStartWindowBgFileString);
    if(customStartWindowBgFile.exists()) {
        centralwidget->setStyleSheet(".QWidget { background-image: url("+QFileInfo(customStartWindowBgFile).absoluteFilePath()+"); background-position: top center; background-origin: content; background-repeat: no-repeat;}");
    } else {
        //if custom bg file could not be found load the big origin file
        centralwidget->setStyleSheet(".QWidget { background-image: url(:/android/android-data/gfx/gui/misc/startwindowbg10_mobile.png); background-position: top center; background-origin: content; background-repeat: no-repeat;}");
    }
    this->showFullScreen();

#else


	this->menubar->setStyleSheet("QMenuBar { background-color: #4B4B4B; font-size:12px; border-width: 0px;} QMenuBar::item { color: #F0F0F0; }");

	centralwidget->setStyleSheet(".QWidget { background-image: url(\""+myAppDataPath+"gfx/gui/misc/startwindowbg.png\"); background-position: bottom center; background-origin: content; background-repeat: no-repeat;}");

	pushButtonStartGame->setStyleSheet("QPushButton { text-align:left; font-weight:bold; padding-left: 1px; padding-bottom: 3px; padding-top: 3px; padding-right: 3px; background-color: #4B4B4B; color: #F0F0F0; font-size:12px; border-width: 0px;}");

    connect( actionAbout_Pokertraining, SIGNAL( triggered() ), this, SLOT( callAboutPokerTrainingDialog() ) );
    connect( actionConfigurePokerTraining, SIGNAL( triggered() ), this, SLOT( callSettingsDialogFromStartwindow() ) );
#endif
#endif

	// 	Dialogs
	myNewGameDialog = new newGameDialogImpl(this, myConfig);
	mySelectAvatarDialog = new selectAvatarDialogImpl(this, myConfig);
	mySettingsDialog = new settingsDialogImpl(this, myConfig, mySelectAvatarDialog, myLog);
	myAboutPokerTrainingDialog = new aboutPokerTrainingImpl(this, myConfig);
	mySettingsDialog->setGuiLog(myGuiLog);

	connect( actionStartGame, SIGNAL( triggered() ), this, SLOT( callNewGameDialog() ) );
	connect( pushButtonStartGame, SIGNAL( clicked() ), this, SLOT( callNewGameDialog() ) );

	this->show();

}

startWindowImpl::~startWindowImpl()
{
}

void startWindowImpl::callNewGameDialog()
{

	//wenn Dialogfenster gezeigt werden soll
	if(myConfig->readConfigInt("ShowGameSettingsDialogOnNewGame")) {

		myNewGameDialog->exec();
		if (myNewGameDialog->result() == QDialog::Accepted ) {
			startNewGame(myNewGameDialog);
		}
	}
	// sonst mit gespeicherten Werten starten
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
		gameData.firstSmallBlind = v->getChangeCompleteBlindsDialog()->spinBox_firstSmallBlind->value();

		if(v->getChangeCompleteBlindsDialog()->radioButton_raiseBlindsAtHands->isChecked()) {
			gameData.raiseIntervalMode = RAISE_ON_HANDNUMBER;
			gameData.raiseSmallBlindEveryHandsValue = v->getChangeCompleteBlindsDialog()->spinBox_raiseSmallBlindEveryHands->value();
		} else {
			gameData.raiseIntervalMode = RAISE_ON_MINUTES;
			gameData.raiseSmallBlindEveryMinutesValue = v->getChangeCompleteBlindsDialog()->spinBox_raiseSmallBlindEveryMinutes->value();
		}

		if(v->getChangeCompleteBlindsDialog()->radioButton_alwaysDoubleBlinds->isChecked()) {
			gameData.raiseMode = DOUBLE_BLINDS;
		} else {
			gameData.raiseMode = MANUAL_BLINDS_ORDER;
			list<int> tempBlindList;
			int i;
			bool ok = true;
			for(i=0; i<v->getChangeCompleteBlindsDialog()->listWidget_blinds->count(); i++) {
				tempBlindList.push_back(v->getChangeCompleteBlindsDialog()->listWidget_blinds->item(i)->text().toInt(&ok,10));
			}
			gameData.manualBlindsList = tempBlindList;

			if(v->getChangeCompleteBlindsDialog()->radioButton_afterThisAlwaysDoubleBlinds->isChecked()) {
				gameData.afterManualBlindsMode = AFTERMB_DOUBLE_BLINDS;
			} else {
				if(v->getChangeCompleteBlindsDialog()->radioButton_afterThisAlwaysRaiseAbout->isChecked()) {
					gameData.afterManualBlindsMode = AFTERMB_RAISE_ABOUT;
					gameData.afterMBAlwaysRaiseValue = v->getChangeCompleteBlindsDialog()->spinBox_afterThisAlwaysRaiseValue->value();
				} else {
					gameData.afterManualBlindsMode = AFTERMB_STAY_AT_LAST_BLIND;
				}
			}
		}

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

		if(myConfig->readConfigInt("RaiseBlindsAtHands")) {
			gameData.raiseIntervalMode = RAISE_ON_HANDNUMBER;
			gameData.raiseSmallBlindEveryHandsValue = myConfig->readConfigInt("RaiseSmallBlindEveryHands");
		} else {
			gameData.raiseIntervalMode = RAISE_ON_MINUTES;
			gameData.raiseSmallBlindEveryMinutesValue = myConfig->readConfigInt("RaiseSmallBlindEveryMinutes");
		}

		if(myConfig->readConfigInt("AlwaysDoubleBlinds")) {
			gameData.raiseMode = DOUBLE_BLINDS;
		} else {
			gameData.raiseMode = MANUAL_BLINDS_ORDER;
			gameData.manualBlindsList = myConfig->readConfigIntList("ManualBlindsList");

			if(myConfig->readConfigInt("AfterMBAlwaysDoubleBlinds")) {
				gameData.afterManualBlindsMode = AFTERMB_DOUBLE_BLINDS;
			} else {
				if(myConfig->readConfigInt("AfterMBAlwaysRaiseAbout")) {
					gameData.afterManualBlindsMode = AFTERMB_RAISE_ABOUT;
					gameData.afterMBAlwaysRaiseValue = myConfig->readConfigInt("AfterMBAlwaysRaiseValue");
				} else {
					gameData.afterManualBlindsMode = AFTERMB_STAY_AT_LAST_BLIND;
				}
			}
		}
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



void startWindowImpl::callAboutPokerTrainingDialog()
{
	myAboutPokerTrainingDialog->exec();
}

void startWindowImpl::callSettingsDialogFromStartwindow()
{
	callSettingsDialog(false);
}

void startWindowImpl::callSettingsDialog(bool ingame)
{
	mySettingsDialog->exec(ingame);

	if (mySettingsDialog->result() == QDialog::Accepted && mySettingsDialog->getSettingsCorrect()) {
		myGuiInterface->getW()->applySettings(mySettingsDialog);
	}
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
