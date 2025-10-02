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
    m_session = std::make_unique<pkt::core::Session>(m_events);
    m_pokerTableWindow = std::make_unique<PokerTableWindow>(m_session.get());
    m_bridge = std::make_unique<GuiBridgeWidgets>(m_session.get(), m_pokerTableWindow.get());

    m_bridge->connectEventsToUi(m_events);
}

GuiAppController::GuiAppController(std::shared_ptr<pkt::core::ServiceContainer> services)
    : m_services(std::move(services))
{
    m_session = std::make_unique<pkt::core::Session>(m_events, m_services);
    m_pokerTableWindow = std::make_unique<PokerTableWindow>(m_session.get());
    m_bridge = std::make_unique<GuiBridgeWidgets>(m_session.get(), m_pokerTableWindow.get());

    m_bridge->connectEventsToUi(m_events);
}

StartWindow* GuiAppController::createMainWindow()
{
    if (m_services)
    {
        auto* w = new StartWindow(m_pokerTableWindow.get(), m_session.get(), m_services, nullptr);
        if (m_pokerTableWindow) m_pokerTableWindow->hide();
        return w;
    }
    else
    {
        auto* w = new StartWindow(m_pokerTableWindow.get(), m_session.get(), nullptr);
        if (m_pokerTableWindow) m_pokerTableWindow->hide();
        return w;
    }
}

GuiAppController::~GuiAppController() = default;
} // namespace pkt::ui::qtwidgets
