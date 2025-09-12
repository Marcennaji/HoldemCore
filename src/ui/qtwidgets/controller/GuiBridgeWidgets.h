#pragma once

#include <QObject>
#include <core/engine/GameEvents.h>

class PokerTableWindow;

namespace pkt::core
{
class SessionFsm;
}
namespace pkt::ui::qtwidgets
{
class PokerTableWindow;

class GuiBridgeWidgets
{
  public:
    GuiBridgeWidgets(pkt::core::SessionFsm* session, PokerTableWindow* pokerTableWindow);
    void connectEventsToUi(pkt::core::GameEvents& events);

  private:
    void connectSignalsFromUi();

    PokerTableWindow* m_tableWindow = nullptr;
    pkt::core::SessionFsm* m_session = nullptr;
};

} // namespace pkt::ui::qtwidgets
