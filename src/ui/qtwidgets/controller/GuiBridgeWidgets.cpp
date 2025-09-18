#include "GuiBridgeWidgets.h"
#include "core/engine/GameEvents.h"
#include "core/session/Session.h"
#include "ui/qtwidgets/poker_ui/PokerTableWindow.h"

using namespace pkt::core;

namespace pkt::ui::qtwidgets
{

GuiBridgeWidgets::GuiBridgeWidgets(pkt::core::Session* session, PokerTableWindow* pokerTableWindow)
    : mySession(session), myTableWindow(pokerTableWindow)
{
}
void GuiBridgeWidgets::connectSignalsFromUi()
{
    // Connect UI signals to session methods
    // TODO: Implement signal-slot connections here
}

void GuiBridgeWidgets::connectEventsToUi(pkt::core::GameEvents& events)
{
    // Connect session events to UI update methods
    // TODO: Implement event-handler connections here
}
} // namespace pkt::ui::qtwidgets
