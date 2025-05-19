// GuiAppController.h
#pragma once

#include <core/engine/GameEvents.h>
#include <core/interfaces/ILogger.h>

#include <QString>
#include <memory>

class StartWindow;
class SqliteLogStore;
class Session;
class GuiWrapper;

class GuiAppController
{
  public:
    GuiAppController(ILogger* logger, const QString& appPath, const QString& logPath, const QString& userDataPath);
    ~GuiAppController();

    StartWindow* createMainWindow();

  private:
    QString myAppDataPath;
    QString myLogPath;
    QString myUserDataPath;
    std::unique_ptr<SqliteLogStore> myGameActionslogger;
    std::unique_ptr<GuiWrapper> myGui;
    std::unique_ptr<Session> mySession;
    std::unique_ptr<GameEvents> myEvents;
};
