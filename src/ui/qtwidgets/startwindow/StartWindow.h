// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#pragma once

#include <assert.h>
#include <memory>

#include <core/engine/EngineDefs.h>
#include <core/session/SessionFsm.h>
#include "ui_StartWindow.h"

#include <QtWidgets/QMainWindow>

namespace pkt::ui::qtwidgets
{
class PokerTableWindow;
class StartWindow : public QMainWindow, public Ui::StartWindow
{
    Q_OBJECT
  public:
    StartWindow(const QString& appDataPath, PokerTableWindow* tableWindow, pkt::core::SessionFsm* session,
                QWidget* parent);
    ~StartWindow();

    void setSession(pkt::core::SessionFsm* session) { mySession = session; }
    pkt::core::SessionFsm* getSession()
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

    std::shared_ptr<PokerTableWindow> myPokerTableWindow;
    pkt::core::SessionFsm* mySession;
};

} // namespace pkt::ui::qtwidgets
