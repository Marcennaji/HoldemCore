// GuiAppController.h
#pragma once

#include <core/engine/GameEvents.h>

#include <core/session/Session.h>

#include <QString>
#include <memory>

namespace pkt::core
{
class Session;
class ServiceContainer;
} // namespace pkt::core

namespace pkt::ui::qtwidgets
{
class PokerTableWindow;
class GuiBridgeWidgets;
class StartWindow;
class GuiAppController
{
  public:
    GuiAppController();
    GuiAppController(std::shared_ptr<pkt::core::ServiceContainer> services);
    ~GuiAppController();

    StartWindow* createMainWindow();

  private:
    std::shared_ptr<pkt::core::ServiceContainer> myServices;
    std::unique_ptr<PokerTableWindow> myPokerTableWindow;
    std::unique_ptr<pkt::core::Session> mySession;
    pkt::core::GameEvents myEvents;
    std::unique_ptr<GuiBridgeWidgets> myBridge;
};
} // namespace pkt::ui::qtwidgets
