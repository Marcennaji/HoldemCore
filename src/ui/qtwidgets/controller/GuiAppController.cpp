// GuiAppController.cpp
#include "GuiAppController.h"
#include <core/session/Session.h>
#include <infra/persistence/SqliteLogStore.h>
#include <ui/qtwidgets/gametable/GameTableWindow.h>
#include <ui/qtwidgets/startwindow/StartWindow.h>

#include "GuiBridgeWidgets.h"

GuiAppController::GuiAppController(const QString& app, const QString& log, const QString& user)
    : myAppDataPath(app), myLogPath(log), myUserDataPath(user)
{
    myDbStatisticsLogger = std::make_unique<pkt::infra::SqliteLogStore>(myLogPath.toStdString());
    myDbStatisticsLogger->init();

    myGameTableWindow = std::make_unique<GameTableWindow>(myUserDataPath.toStdString());
    myBridge = std::make_unique<GuiBridgeWidgets>(myGameTableWindow.get());
    myBridge->connectTo(myEvents);

    mySession = std::make_unique<pkt::core::Session>(myEvents);
}

StartWindow* GuiAppController::createMainWindow()
{
    return new StartWindow(myAppDataPath, myGameTableWindow.get(), mySession.get(), nullptr);
}

GuiAppController::~GuiAppController() = default;
