// GuiAppController.h
#pragma once

#include <core/engine/EngineServices.h>

#include <memory>
#include <QString>

class startWindowImpl;
class Log;
class Session;
class GuiWrapper;

class GuiAppController {
public:
    GuiAppController(EngineServices& svc, const QString& appPath, const QString& logPath, const QString& userDataPath);
    ~GuiAppController();

    startWindowImpl* createMainWindow();

private:
    QString appDataPath, logPath, userDataPath;
    std::unique_ptr<Log> logger;
    std::unique_ptr<GuiWrapper> gui;
    std::unique_ptr<Session> session;
};
