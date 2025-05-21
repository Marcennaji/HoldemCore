#include "GuiBridgeWidgets.h"
#include "ui/qtwidgets/gametable/GameTableWindow.h"

GuiBridgeWidgets::GuiBridgeWidgets(GameTableWindow* table, QObject* parent) : QObject(parent), myGameTableWindow(table)
{
}

void GuiBridgeWidgets::connectTo(GameEvents& events)
{
    events.onPotUpdated = [this](int pot)
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this]() { myGameTableWindow->refreshPot(); }, Qt::DirectConnection);
    };

    events.onRefreshCash = [this]()
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this]() { myGameTableWindow->refreshCash(); }, Qt::DirectConnection);
    };

    events.onRefreshPlayerName = [this]()
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this]() { myGameTableWindow->refreshPlayerName(); }, Qt::DirectConnection);
    };
    events.onRefreshSet = [this]()
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this]() { myGameTableWindow->refreshSet(); }, Qt::DirectConnection);
    };
    events.onHideHoleCards = [this]()
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this]() { myGameTableWindow->hideHoleCards(); }, Qt::DirectConnection);
    };
    events.onDealHoleCards = [this]()
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this]() { myGameTableWindow->dealHoleCards(); }, Qt::DirectConnection);
    };
    events.onDealBettingRoundCards = [this](int bettingRoundId)
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this, bettingRoundId]() { myGameTableWindow->dealBettingRoundCards(bettingRoundId); },
            Qt::DirectConnection);
    };
    events.onRefreshAction = [this](int playerId, int playerAction)
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this, playerId, playerAction]()
            { myGameTableWindow->refreshAction(playerId, playerAction); }, Qt::DirectConnection);
    };
}
