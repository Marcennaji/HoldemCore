// GuiAppController.cpp
#include "GuiAppController.h"
#include <app/session.h>
#include <infra/persistence/SqliteLogStore.h>
#include <ui/qtwidgets/guiwrapper.h>
#include <ui/qtwidgets/startwindow/StartWindow.h>

GuiAppController::GuiAppController(ILogger* logger, const QString& app, const QString& log, const QString& user)
    : appDataPath(app), logPath(log), userDataPath(user)
{
    gameActionslogger = std::make_unique<SqliteLogStore>(logPath.toStdString());
    gui = std::make_unique<GuiWrapper>(userDataPath.toStdString(), nullptr);
    session = std::make_unique<Session>(logger, gui.get(), gameActionslogger.get(), gameActionslogger.get(),
                                        gameActionslogger.get());
    session->init();
    gameActionslogger->init();
}

StartWindow* GuiAppController::createMainWindow()
{
    return new StartWindow(appDataPath, gui.get(), session.get(), nullptr);
}

GuiAppController::~GuiAppController() = default;
