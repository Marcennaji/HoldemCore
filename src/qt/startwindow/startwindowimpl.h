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
#ifndef STARTWINDOWIMPL_H
#define STARTWINDOWIMPL_H

#include <memory>
#include <assert.h>

#include "ui_startwindow.h"

#include "game_defs.h"
#include <QtWidgets/QMessageBox>
#include "session.h"

class GuiInterface;
class GuiWrapper;
class Game;

class ConfigFile;
class gameTableImpl;
class newGameDialogImpl;
class guiLog;
class Log;

class startWindowImpl: public QMainWindow, public Ui::startWindow
{
	Q_OBJECT
public:
	startWindowImpl(ConfigFile *c, Log *l);
	~startWindowImpl();

	void setSession(std::shared_ptr<Session> session) {
		mySession = session;
	}
	std::shared_ptr<Session> getSession() {
		assert(mySession.get());
		return mySession;
	}
	void setGuiLog(guiLog* l) {
		myGuiLog = l;
	}

	//	void keyPressEvent( QKeyEvent *);
	bool eventFilter(QObject *obj, QEvent *event);

signals:


public slots:

	void callNewGameDialog();

	void startNewGame(newGameDialogImpl* =0);

private:
	ConfigFile *myConfig;
	guiLog *myGuiLog;
	Log *myLog;

	std::shared_ptr<GuiInterface> myGuiInterface;
	std::shared_ptr<Session> mySession;
	//std::shared_ptr<GuiInterface> myServerGuiInterface;

	// 	Dialogs
	newGameDialogImpl *myNewGameDialog;
	startWindowImpl *myStartWindow;
 
	friend class GuiWrapper;
};

#endif
