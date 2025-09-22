// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#pragma once

#include <assert.h>
#include <memory>

#include <core/engine/EngineDefs.h>
#include <core/session/Session.h>
#include "ui_StartWindow.h"

#include <QtWidgets/QMainWindow>

namespace pkt::core
{
class ServiceContainer;
}

namespace pkt::ui::qtwidgets
{
class PokerTableWindow;
class StartWindow : public QMainWindow, public Ui::StartWindow
{
    Q_OBJECT
  public:
    StartWindow(PokerTableWindow* tableWindow, pkt::core::Session* session, QWidget* parent = nullptr);

    StartWindow(PokerTableWindow* tableWindow, pkt::core::Session* session,
                std::shared_ptr<pkt::core::ServiceContainer> services, QWidget* parent = nullptr);

    ~StartWindow();

    void setSession(pkt::core::Session* session) { mySession = session; }
    pkt::core::Session* getSession()
    {
        assert(mySession != nullptr);
        return mySession;
    }

    bool eventFilter(QObject* obj, QEvent* event);

  signals:

  public slots:

    void startNewGame();

  private:
    std::shared_ptr<PokerTableWindow> myPokerTableWindow;
    pkt::core::Session* mySession;

    void ensureServicesInitialized();

    std::shared_ptr<pkt::core::ServiceContainer> myServices;
};

} // namespace pkt::ui::qtwidgets
