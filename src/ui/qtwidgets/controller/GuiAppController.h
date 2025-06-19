// GuiAppController.h
#pragma once

#include <core/engine/GameEvents.h>

#include <core/session/Session.h>
#include <infra/persistence/SqliteLogStore.h>

#include <QString>
#include <memory>

class StartWindow;
class SqliteLogStore;
class Session;
class GameTableWindow;
class GuiBridgeWidgets;

class GuiAppController
{
  public:
    GuiAppController(const QString& appPath, const QString& logPath, const QString& userDataPath);
    ~GuiAppController();

    StartWindow* createMainWindow();

  private:
    QString myAppDataPath;
    QString myLogPath;
    QString myUserDataPath;
    std::unique_ptr<pkt::infra::SqliteLogStore> myDbStatisticsLogger;
    std::unique_ptr<GameTableWindow> myGameTableWindow;
    std::unique_ptr<pkt::core::Session> mySession;
    pkt::core::GameEvents myEvents;
    std::unique_ptr<GuiBridgeWidgets> myBridge;
};
