// GuiAppController.cpp
#include "GuiAppController.h"
#include <core/services/ServiceContainer.h>
#include <core/session/Session.h>
#include <infra/persistence/SqliteDb.h>
#include <ui/qtwidgets/poker_ui/PokerTableWindow.h>
#include <ui/qtwidgets/startwindow/StartWindow.h>

#include "GuiBridgeWidgets.h"
namespace pkt::ui::qtwidgets
{

GuiAppController::GuiAppController(const QString& app) : myAppDataPath(app)
{
    mySession = std::make_unique<pkt::core::Session>(myEvents);
    myPokerTableWindow = std::make_unique<PokerTableWindow>(mySession.get());
    myBridge = std::make_unique<GuiBridgeWidgets>(mySession.get(), myPokerTableWindow.get());

    myBridge->connectEventsToUi(myEvents);
}

GuiAppController::GuiAppController(const QString& app, std::shared_ptr<pkt::core::ServiceContainer> services)
    : myAppDataPath(app), myServices(std::move(services))
{
    mySession = std::make_unique<pkt::core::Session>(myEvents, myServices);
    myPokerTableWindow = std::make_unique<PokerTableWindow>(mySession.get());
    myBridge = std::make_unique<GuiBridgeWidgets>(mySession.get(), myPokerTableWindow.get());

    myBridge->connectEventsToUi(myEvents);
}

StartWindow* GuiAppController::createMainWindow()
{
    if (myServices)
    {
        return new StartWindow(myAppDataPath, myPokerTableWindow.get(), mySession.get(), myServices, nullptr);
    }
    else
    {
        return new StartWindow(myAppDataPath, myPokerTableWindow.get(), mySession.get(), nullptr);
    }
}

GuiAppController::~GuiAppController() = default;
} // namespace pkt::ui::qtwidgets
