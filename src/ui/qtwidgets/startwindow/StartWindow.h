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
#ifndef STARTWINDOWIMPL_H
#define STARTWINDOWIMPL_H

#include <assert.h>
#include <memory>

#include "ui_startwindow.h"

#include <core/engine/EngineDefs.h>
#include <core/session/Session.h>

#include <QtWidgets/QMainWindow>

class GameTableWindow;

class StartWindow : public QMainWindow, public Ui::startWindow
{
    Q_OBJECT
  public:
    StartWindow(const QString& appDataPath, GameTableWindow* tableWindow, pkt::core::Session* session, QWidget* parent);
    ~StartWindow();

    void setSession(pkt::core::Session* session) { mySession = session; }
    pkt::core::Session* getSession()
    {
        assert(mySession != nullptr);
        return mySession;
    }

    //	void keyPressEvent( QKeyEvent *);
    bool eventFilter(QObject* obj, QEvent* event);

  signals:

  public slots:

    void startNewGame();

  private:
    QString myAppDataPath;

    std::shared_ptr<GameTableWindow> myGameTableWindow;
    pkt::core::Session* mySession;

    friend class GuiWrapper;
};

#endif
