// GuiAppController.cpp
#include "GuiAppController.h"
#include <core/session/Session.h>
#include <infra/persistence/SqliteDb.h>
#include <ui/qtwidgets/poker_ui/PokerTableWindow.h>
#include <ui/qtwidgets/startwindow/StartWindow.h>

#include "GuiBridgeWidgets.h"
namespace pkt::ui::qtwidgets
{

GuiAppController::GuiAppController(const QString& app) : myAppDataPath(app)
{
    mySession = std::make_shared<pkt::core::Session>(myEvents);
    myPokerTableWindow = std::make_unique<PokerTableWindow>(mySession);
    myBridge = std::make_unique<GuiBridgeWidgets>(mySession, myPokerTableWindow.get());

    myBridge->connectEventsToUi(myEvents);
}

StartWindow* GuiAppController::createMainWindow()
{
    return new StartWindow(myAppDataPath, myPokerTableWindow.get(), mySession.get(), nullptr);
}

GuiAppController::~GuiAppController() = default;
} // namespace pkt::ui::qtwidgets
