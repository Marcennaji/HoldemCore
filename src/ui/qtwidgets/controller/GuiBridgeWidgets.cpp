#include "GuiBridgeWidgets.h"
#include "ui/qtwidgets/gametable/GameTableWindow.h"

GuiBridgeWidgets::GuiBridgeWidgets(GameTableWindow* table, QObject* parent) : QObject(parent), myGameTableWindow(table)
{
}

void GuiBridgeWidgets::connectTo(GameEvents& events)
{
    // Update pot display
    events.onPotUpdated = [this](int pot)
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this]() { myGameTableWindow->refreshPot(); }, Qt::QueuedConnection);
    };

    // Update cash display
    events.onRefreshCash = [this]()
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this]() { myGameTableWindow->refreshCash(); }, Qt::QueuedConnection);
    };

    events.onRefreshPlayerName = [this]()
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this]() { myGameTableWindow->refreshPlayerName(); }, Qt::QueuedConnection);
    };
    events.onRefreshSet = [this]()
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this]() { myGameTableWindow->refreshSet(); }, Qt::QueuedConnection);
    };
}
