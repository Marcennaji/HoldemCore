// GuiAppController.cpp
#include "GuiAppController.h"
#include <core/session/Session.h>
#include <infra/persistence/SqliteLogStore.h>
#include <ui/qtwidgets/gametable/GameTableWindow.h>
#include <ui/qtwidgets/startwindow/StartWindow.h>

#include "GuiBridgeWidgets.h"

GuiAppController::GuiAppController(ILogger* logger, const QString& app, const QString& log, const QString& user)
    : myAppDataPath(app), myLogPath(log), myUserDataPath(user)
{
    assert(logger != nullptr);

    myEvents = std::make_unique<GameEvents>();

    myGameActionslogger = std::make_unique<SqliteLogStore>(myLogPath.toStdString());
    myGameActionslogger->init();

    myGameTableWindow = std::make_unique<GameTableWindow>(myUserDataPath.toStdString());
    myBridge = std::make_unique<GuiBridgeWidgets>(myGameTableWindow.get());
    myBridge->connectTo(*myEvents);

    mySession = std::make_unique<Session>(myEvents.get(), logger, myGameActionslogger.get(), myGameActionslogger.get(),
                                          myGameActionslogger.get());
}

StartWindow* GuiAppController::createMainWindow()
{
    return new StartWindow(myAppDataPath, myGameTableWindow.get(), mySession.get(), nullptr);
}

GuiAppController::~GuiAppController() = default;
