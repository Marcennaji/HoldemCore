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
    GuiBridgeWidgets(pkt::core::Session* session, PokerTableWindow* pokerTableWindow);
    void connectEventsToUi(pkt::core::GameEvents& events);

  private:
    void connectSignalsFromUi();

    PokerTableWindow* myTableWindow = nullptr;
    pkt::core::Session* mySession = nullptr;
};

} // namespace pkt::ui::qtwidgets
