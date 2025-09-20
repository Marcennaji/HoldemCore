// PokerTraining — Texas Hold'em simulator
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
    StartWindow(const QString& appDataPath, PokerTableWindow* tableWindow, pkt::core::Session* session,
                QWidget* parent);

    /// Constructor with ServiceContainer dependency injection
    StartWindow(const QString& appDataPath, PokerTableWindow* tableWindow, pkt::core::Session* session,
                std::shared_ptr<pkt::core::ServiceContainer> services, QWidget* parent);

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

    std::shared_ptr<PokerTableWindow> myPokerTableWindow;
    pkt::core::Session* mySession;

    /// Ensures services are initialized (lazy initialization)
    void ensureServicesInitialized();

    /// ServiceContainer for dependency injection
    std::shared_ptr<pkt::core::ServiceContainer> myServices;
};

} // namespace pkt::ui::qtwidgets
