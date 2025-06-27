#pragma once

#include <QObject>
#include <core/engine/GameEvents.h>

class PokerTableWindow;

namespace pkt::core
{
class Session;
}
namespace pkt::ui::qtwidgets
{
class PokerTableWindow;

class GuiBridgeWidgets
{
  public:
    GuiBridgeWidgets(std::shared_ptr<pkt::core::Session> session, PokerTableWindow* pokerTableWindow);
    void connectEventsToUi(pkt::core::GameEvents& events);

  private:
    void connectSignalsFromUi();

    PokerTableWindow* m_tableWindow = nullptr;
    std::shared_ptr<pkt::core::Session> m_session = nullptr;
};

} // namespace pkt::ui::qtwidgets
