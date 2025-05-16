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

#include <assert.h>
#include <memory>

#include "ui_startwindow.h"

#include <core/engine/EngineDefs.h>
#include <core/session/Session.h>

#include <QtWidgets/QMainWindow>

class IGui;
class GuiWrapper;
class Game;

class GameTableWindow;
class GuiDisplayGameActions;
class SqliteLogStore;

class StartWindow : public QMainWindow, public Ui::startWindow
{
    Q_OBJECT
  public:
    StartWindow(const QString& appDataPath, IGui* gui, Session* session, QWidget* parent);
    ~StartWindow();

    void setSession(std::shared_ptr<Session> session) { mySession = session; }
    std::shared_ptr<Session> getSession()
    {
        assert(mySession.get());
        return mySession;
    }
    void setGuiLog(GuiDisplayGameActions* l) { myGuiLog = l; }

    //	void keyPressEvent( QKeyEvent *);
    bool eventFilter(QObject* obj, QEvent* event);

  signals:

  public slots:

    void startNewGame();

  private:
    QString myAppDataPath;

    GuiDisplayGameActions* myGuiLog;

    std::shared_ptr<IGui> myGuiInterface;
    std::shared_ptr<Session> mySession;

    friend class GuiWrapper;
};

#endif
