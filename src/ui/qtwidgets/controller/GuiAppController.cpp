// GuiAppController.cpp
#include "GuiAppController.h"
#include <core/services/ServiceContainer.h>
#include <core/session/Session.h>
#include <infra/persistence/SqliteDb.h>
#include <ui/qtwidgets/windows/PokerTableWindow.h>
#include <ui/qtwidgets/windows/StartWindow.h>

#include "GuiBridgeWidgets.h"
namespace pkt::ui::qtwidgets
{

GuiAppController::GuiAppController()
{
    mySession = std::make_unique<pkt::core::Session>(myEvents);
    myPokerTableWindow = std::make_unique<PokerTableWindow>(mySession.get());
    myBridge = std::make_unique<GuiBridgeWidgets>(mySession.get(), myPokerTableWindow.get());

    myBridge->connectEventsToUi(myEvents);
}

GuiAppController::GuiAppController(std::shared_ptr<pkt::core::ServiceContainer> services)
    : myServices(std::move(services))
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
        auto* w = new StartWindow(myPokerTableWindow.get(), mySession.get(), myServices, nullptr);
        if (myPokerTableWindow) myPokerTableWindow->hide();
        return w;
    }
    else
    {
        auto* w = new StartWindow(myPokerTableWindow.get(), mySession.get(), nullptr);
        if (myPokerTableWindow) myPokerTableWindow->hide();
        return w;
    }
}

GuiAppController::~GuiAppController() = default;
} // namespace pkt::ui::qtwidgets
