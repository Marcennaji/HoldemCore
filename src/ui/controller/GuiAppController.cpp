// GuiAppController.cpp
#include "GuiAppController.h"
#include <ui/startwindow/startwindowimpl.h>
#include <ui/guiwrapper.h>
#include <core/engine/Log.h>
#include <app/session.h>

GuiAppController::GuiAppController(EngineServices& svc,const QString& app, const QString& log, const QString& user)
    : appDataPath(app), logPath(log), userDataPath(user)
{
    logger = std::make_unique<Log>(logPath.toStdString());
    gui = std::make_unique<GuiWrapper>(userDataPath.toStdString(), nullptr);
    session = std::make_unique<Session>(svc, gui.get(), logger.get());
    session->init();
    logger->init();
}

startWindowImpl* GuiAppController::createMainWindow() {
    return new startWindowImpl(appDataPath, gui.get(), session.get(), nullptr);
}

GuiAppController::~GuiAppController() = default;
