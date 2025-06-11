#include "GuiBridgeWidgets.h"
#include "ui/qtwidgets/gametable/GameTableWindow.h"

GuiBridgeWidgets::GuiBridgeWidgets(GameTableWindow* table, QObject* parent) : QObject(parent), myGameTableWindow(table)
{
}

void GuiBridgeWidgets::connectTo(pkt::core::GameEvents& events)
{
    events.onInitializeGui = [this](int gameSpeed)
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this, gameSpeed]() { myGameTableWindow->initializeGui(gameSpeed); },
            Qt::DirectConnection);
    };
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
    events.onShowHoleCards = [this](unsigned playerId)
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this, playerId]() { myGameTableWindow->showHoleCards(playerId); },
            Qt::DirectConnection);
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

    events.onNextBettingRoundInitializeGui = [this]()
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this]() { myGameTableWindow->nextBettingRoundInitializeGui(); }, Qt::DirectConnection);
    };

    events.onRefreshTableDescriptiveLabels = [this](int state)
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this, state]() { myGameTableWindow->refreshTableDescriptiveLabels(state); },
            Qt::DirectConnection);
    };
    events.onRefreshPlayersActiveInactiveStyles = [this](int playerId, int status)
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this, playerId, status]()
            { myGameTableWindow->refreshPlayersActiveInactiveStyles(playerId, status); }, Qt::DirectConnection);
    };
    events.onActivePlayerActionDone = [this]()
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this]() { myGameTableWindow->activePlayerActionDone(); }, Qt::DirectConnection);
    };
    events.onDoHumanAction = [this]()
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this]() { myGameTableWindow->doHumanAction(); }, Qt::DirectConnection);
    };
    events.onFlipHoleCardsAllIn = [this]()
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this]() { myGameTableWindow->flipHoleCardsAllIn(); }, Qt::DirectConnection);
    };
    events.onBettingRoundAnimation = [this](int bettingRoundId)
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this, bettingRoundId]() { myGameTableWindow->bettingRoundAnimation(bettingRoundId); },
            Qt::DirectConnection);
    };
    events.onPreflopAnimation = [this]()
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this]() { myGameTableWindow->preflopAnimation1(); }, Qt::DirectConnection);
    };
    events.onFlopAnimation = [this]()
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this]() { myGameTableWindow->flopAnimation1(); }, Qt::DirectConnection);
    };
    events.onTurnAnimation = [this]()
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this]() { myGameTableWindow->turnAnimation1(); }, Qt::DirectConnection);
    };
    events.onRiverAnimation = [this]()
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this]() { myGameTableWindow->riverAnimation1(); }, Qt::DirectConnection);
    };
    events.onPostRiverAnimation = [this]()
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this]() { myGameTableWindow->postRiverAnimation1(); }, Qt::DirectConnection);
    };
    events.onPostRiverRunAnimation = [this]()
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this]() { myGameTableWindow->postRiverRunAnimation1(); }, Qt::DirectConnection);
    };
    events.onPauseHand = [this]()
    {
        QMetaObject::invokeMethod(
            myGameTableWindow, [this]() { myGameTableWindow->breakButtonClicked(); }, Qt::DirectConnection);
    };
}
