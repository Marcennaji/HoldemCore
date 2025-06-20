// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#ifndef STARTWINDOWIMPL_H
#define STARTWINDOWIMPL_H

#include <assert.h>
#include <memory>

#include <core/engine/EngineDefs.h>
#include <core/session/Session.h>
#include "ui_StartWindow.h"

#include <QtWidgets/QMainWindow>

class GameTableWindow;

class StartWindow : public QMainWindow, public Ui::StartWindow
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
