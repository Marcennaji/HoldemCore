// GuiAppController.cpp
#include "GuiAppController.h"
#include <core/session/Session.h>
#include <infra/persistence/SqliteLogStore.h>
#include <ui/qtwidgets/gametable/GameTableWindow.h>
#include <ui/qtwidgets/guiwrapper.h>
#include <ui/qtwidgets/startwindow/StartWindow.h>

#include "GuiBridgeWidgets.h"

GuiAppController::GuiAppController(ILogger* logger, const QString& app, const QString& log, const QString& user)
    : myAppDataPath(app), myLogPath(log), myUserDataPath(user)
{
    assert(logger != nullptr);

    myGameActionslogger = std::make_unique<SqliteLogStore>(myLogPath.toStdString());
    myGameActionslogger->init();
    myGui = std::make_unique<GuiWrapper>(myUserDataPath.toStdString(), nullptr);
    myEvents = std::make_unique<GameEvents>();
    myBridge = std::make_unique<GuiBridgeWidgets>(static_cast<GameTableWindow*>(myGui->getGameTableWindow()));
    myBridge->connectTo(*myEvents);
    mySession = std::make_unique<Session>(myEvents.get(), logger, myGameActionslogger.get(), myGameActionslogger.get(),
                                          myGameActionslogger.get());
}

StartWindow* GuiAppController::createMainWindow()
{
    return new StartWindow(myAppDataPath, myGui.get(), mySession.get(), nullptr);
}

GuiAppController::~GuiAppController() = default;
