// GuiAppController.h
#pragma once

#include <core/interfaces/ILogger.h>

#include <memory>
#include <QString>

class startWindowImpl;
class SqliteLogStore;
class Session;
class GuiWrapper;

class GuiAppController {
public:
    GuiAppController(ILogger * logger, const QString& appPath, const QString& logPath, const QString& userDataPath);
    ~GuiAppController();

    startWindowImpl* createMainWindow();

private:
    QString appDataPath, logPath, userDataPath;
    std::unique_ptr<SqliteLogStore> gameActionslogger;
    std::unique_ptr<GuiWrapper> gui;
    std::unique_ptr<Session> session;
};
