// GuiAppController.cpp
#include "GuiAppController.h"
#include <ui/qtwidgets/startwindow/startwindowimpl.h>
#include <ui/qtwidgets/guiwrapper.h>
#include <core/engine/Log.h>
#include <app/session.h>

GuiAppController::GuiAppController(ILogger * logger,const QString& app, const QString& log, const QString& user)
    : appDataPath(app), logPath(log), userDataPath(user)
{
    gameActionslogger = std::make_unique<Log>(logPath.toStdString());
    gui = std::make_unique<GuiWrapper>(userDataPath.toStdString(), nullptr);
    session = std::make_unique<Session>(logger, gui.get(), gameActionslogger.get());
    session->init();
    gameActionslogger->init();
}

startWindowImpl* GuiAppController::createMainWindow() {
    return new startWindowImpl(appDataPath, gui.get(), session.get(), nullptr);
}

GuiAppController::~GuiAppController() = default;
