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
    std::shared_ptr<pkt::core::ServiceContainer> m_services;
    std::unique_ptr<PokerTableWindow> m_pokerTableWindow;
    std::unique_ptr<pkt::core::Session> m_session;
    pkt::core::GameEvents m_events;
    std::unique_ptr<GuiBridgeWidgets> m_bridge;
};
} // namespace pkt::ui::qtwidgets
