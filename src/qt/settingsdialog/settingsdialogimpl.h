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
#ifndef SETTINGSDIALOGIMPL_H
#define SETTINGSDIALOGIMPL_H

#ifdef GUI_800x480
#include "ui_settingsdialog_800x480.h"
#else
#include "ui_settingsdialog.h"
#endif
#include <qt/settingsdialog/selectavatardialog/selectavatardialogimpl.h>
#include <qt/settingsdialog/manualblindsorderdialog/manualblindsorderdialogimpl.h>

#include <QtCore>
#include <QtGui>

#include <memory>
#include <vector>
#include <iostream>
#include <engine/log.h>

class ConfigFile;
class selectAvatarDialogImpl;
class manualBlindsOrderDialogImpl;
class MyStyleListItem;
class guiLog;

class settingsDialogImpl: public QDialog, public Ui::settingsDialog
{
	Q_OBJECT
public:
	settingsDialogImpl(QWidget *parent = 0, ConfigFile *c = 0, selectAvatarDialogImpl *s = 0, Log *l=0);

	~settingsDialogImpl();

	void prepareDialog();
	int exec(bool ingame);

	void setPlayerNickIsChanged(bool theValue) {
		playerNickIsChanged = theValue;
	}
	bool getPlayerNickIsChanged() const {
		return playerNickIsChanged;
	}

	bool getSettingsCorrect() const {
		return settingsCorrect;
	}

	void setGuiLog(guiLog* l) {
		myGuiLog = l;
	}

public slots:

	void isAccepted();
	void playerNickChanged() {
		setPlayerNickIsChanged(true);
	};
	void setFlipsidePicFileName();
	void setAvatarFile0();
	void setAvatarFile1();
	void setAvatarFile2();
	void setAvatarFile3();
	void setAvatarFile4();
	void setAvatarFile5();
	void setAvatarFile6();
	void setAvatarFile7();
	void setAvatarFile8();
	void setAvatarFile9();

	void callManualBlindsOrderDialog();
	void resetHumanPlayerStatistics();
	void resetAllPlayersStatistics();

	void callSelectAvatarDialog() {
		mySelectAvatarDialogImpl->exec();
	}

	void setFirstSmallBlindMargin() ;

	void setLanguageChanged(int);

	void setSelectedGameTableStyleActivated();
	void showCurrentGameTableStylePreview();
	void addGameTableStyle();
	void removeGameTableStyle();

	void setSelectedCardDeckStyleActivated();
	void showCurrentCardDeckStylePreview();
	void addCardDeckStyle();
	void removeCardDeckStyle();
	void keyPressEvent ( QKeyEvent * event );

private:

	bool playerNickIsChanged;
	bool settingsCorrect;

	bool myAfterMBAlwaysDoubleBlinds;
	bool myNetAfterMBAlwaysDoubleBlinds;
	bool myAfterMBAlwaysRaiseAbout;
	bool myNetAfterMBAlwaysRaiseAbout;
	bool myAfterMBStayAtLastBlind;
	bool myNetAfterMBStayAtLastBlind;
	int myAfterMBAlwaysRaiseValue;
	int myNetAfterMBAlwaysRaiseValue;
	std::list<int> myManualBlindsList;
	std::list<int> myNetManualBlindsList;
	std::list<std::string> myGameTableStylesList;
	std::list<std::string> myCardDeckStylesList;

	ConfigFile* myConfig;
	Log * myLog;
	guiLog* myGuiLog;
	selectAvatarDialogImpl *mySelectAvatarDialogImpl;
	manualBlindsOrderDialogImpl *myManualBlindsOrderDialog;

	QString myAppDataPath;

	bool languageIsChanged;
	bool calledIngame;
	int changedLanguageIndex;
};

#endif
