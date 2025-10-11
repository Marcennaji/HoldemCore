// GuiAppController.cpp
#include "GuiAppController.h"
#include <core/session/Session.h>
#include <core/engine/EngineFactory.h>
#include <ui/qtwidgets/windows/PokerTableWindow.h>
#include <ui/qtwidgets/windows/StartWindow.h>

#include "GuiBridgeWidgets.h"
namespace pkt::ui::qtwidgets
{

GuiAppController::GuiAppController(std::shared_ptr<pkt::core::Logger> logger,
                                   std::shared_ptr<pkt::core::HandEvaluationEngine> handEvaluationEngine,
                                   std::shared_ptr<pkt::core::PlayersStatisticsStore> statisticsStore,
                                   std::shared_ptr<pkt::core::Randomizer> randomizer)
    : m_logger(std::move(logger)),
      m_handEvaluationEngine(std::move(handEvaluationEngine)),
      m_statisticsStore(std::move(statisticsStore)),
      m_randomizer(std::move(randomizer))
{
    auto engineFactory = std::make_shared<pkt::core::EngineFactory>(m_events, *m_logger, *m_handEvaluationEngine, *m_statisticsStore, *m_randomizer);
    m_session = std::make_unique<pkt::core::Session>(m_events, *engineFactory, *m_logger, *m_handEvaluationEngine, *m_statisticsStore, *m_randomizer);
    m_pokerTableWindow = std::make_unique<PokerTableWindow>(m_session.get());
    m_bridge = std::make_unique<GuiBridgeWidgets>(m_session.get(), m_pokerTableWindow.get());

    m_bridge->connectEventsToUi(m_events);
}

StartWindow* GuiAppController::createMainWindow()
{
    auto* w = new StartWindow(m_pokerTableWindow.get(), m_session.get(), m_randomizer, nullptr);
    if (m_pokerTableWindow) m_pokerTableWindow->hide();
    return w;
}

GuiAppController::~GuiAppController() = default;
} // namespace pkt::ui::qtwidgets
