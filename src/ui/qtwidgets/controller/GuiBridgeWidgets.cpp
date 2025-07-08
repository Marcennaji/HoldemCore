#include "GuiBridgeWidgets.h"
#include "core/engine/GameEvents.h"
#include "core/session/Session.h"
#include "ui/qtwidgets/poker_ui/PokerTableWindow.h"

using namespace pkt::core;

namespace pkt::ui::qtwidgets
{

GuiBridgeWidgets::GuiBridgeWidgets(std::shared_ptr<Session> session, PokerTableWindow* pokerTableWindow)
    : m_session(std::move(session)), m_tableWindow(pokerTableWindow)
{
}
void GuiBridgeWidgets::connectSignalsFromUi()
{
    /*
    QObject::connect(m_tableWindow, &PokerTableWindow::foldClicked, [this]() { m_session->humanPlayerAction_Fold(); });

    QObject::connect(m_tableWindow, &PokerTableWindow::callClicked, [this]() { m_session->humanPlayerAction_Call(); });

    QObject::connect(m_tableWindow, &PokerTableWindow::checkClicked,
                     [this]() { m_session->humanPlayerAction_Check(); });

    QObject::connect(m_tableWindow, &PokerTableWindow::betClicked,
                     [this](int amount) { m_session->humanPlayerAction_Bet(amount); });

    QObject::connect(m_tableWindow, &PokerTableWindow::allInClicked,
                     [this]() { m_session->humanPlayerAction_AllIn(); });
                     */
}

void GuiBridgeWidgets::connectEventsToUi(pkt::core::GameEvents& events)
{
    events.onPotUpdated = [this](int amount)
    {
        if (m_tableWindow)
            m_tableWindow->refreshPot(amount);
    };

    /*
   events.onGameInitialized = [this](int gameSpeed)
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this, gameSpeed]() { myPokerTableWindow->gameInitializeGui(gameSpeed); },
           Qt::DirectConnection);
   };
   events.onPotUpdated = [this](int pot)
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this]() { myPokerTableWindow->refreshPot(); }, Qt::DirectConnection);
   };

   events.onRefreshCash = [this]()
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this]() { myPokerTableWindow->refreshCash(); }, Qt::DirectConnection);
   };

   events.onRefreshPlayerName = [this]()
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this]() { myPokerTableWindow->refreshPlayerName(); }, Qt::DirectConnection);
   };
   events.onRefreshSet = [this]()
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this]() { myPokerTableWindow->refreshSet(); }, Qt::DirectConnection);
   };
   events.onHideHoleCards = [this]()
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this]() { myPokerTableWindow->hideHoleCards(); }, Qt::DirectConnection);
   };
   events.onDealHoleCards = [this]()
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this]() { myPokerTableWindow->dealHoleCards(); }, Qt::DirectConnection);
   };
   events.onShowHoleCards = [this](unsigned playerId)
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this, playerId]() { myPokerTableWindow->showHoleCards(playerId); },
           Qt::DirectConnection);
   };
   events.onDealCommunityCards = [this](int bettingRoundId)
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this, bettingRoundId]() {
   myPokerTableWindow->dealBettingRoundCards(bettingRoundId); }, Qt::DirectConnection);
   };
   events.onPlayerActed = [this](int playerId, int playerAction)
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this, playerId, playerAction]()
           { myPokerTableWindow->refreshAction(playerId, playerAction); }, Qt::DirectConnection);
   };

   events.onNextBettingRoundInitializeGui = [this]()
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this]() { myPokerTableWindow->nextBettingRoundInitializeGui(); },
           Qt::DirectConnection);
   };

   events.onRefreshTableDescriptiveLabels = [this](int state)
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this, state]() { myPokerTableWindow->refreshTableDescriptiveLabels(state); },
           Qt::DirectConnection);
   };
   events.onPlayerStatusChanged = [this](int playerId, int status)
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this, playerId, status]()
           { myPokerTableWindow->refreshPlayersActiveInactiveStyles(playerId, status); }, Qt::DirectConnection);
   };
   events.onActivePlayerActionDone = [this]()
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this]() { myPokerTableWindow->activePlayerActionDone(); }, Qt::DirectConnection);
   };
   events.onAwaitingHumanInput = [this]()
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this]() { myPokerTableWindow->doHumanAction(); }, Qt::DirectConnection);
   };
   events.onFlipHoleCardsAllIn = [this]()
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this]() { myPokerTableWindow->flipHoleCardsAllIn(); }, Qt::DirectConnection);
   };
   events.onBettingRoundAnimation = [this](int bettingRoundId)
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this, bettingRoundId]() {
   myPokerTableWindow->bettingRoundAnimation(bettingRoundId); }, Qt::DirectConnection);
   };
   events.onStartPreflop = [this]()
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this]() { myPokerTableWindow->startPreflop(); }, Qt::DirectConnection);
   };
   events.onStartFlop = [this]()
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this]() { myPokerTableWindow->startFlop(); }, Qt::DirectConnection);
   };
   events.onStartTurn = [this]()
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this]() { myPokerTableWindow->startTurn(); }, Qt::DirectConnection);
   };
   events.onStartRiver = [this]()
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this]() { myPokerTableWindow->startRiver(); }, Qt::DirectConnection);
   };
   events.onStartPostRiver = [this]()
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this]() { myPokerTableWindow->startPostRiver(); }, Qt::DirectConnection);
   };
   events.onShowdownStarted = [this]()
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this]() { myPokerTableWindow->postRiverRunAnimation1(); }, Qt::DirectConnection);
   };
   events.onPauseHand = [this]()
   {
       QMetaObject::invokeMethod(
           myPokerTableWindow, [this]() { myPokerTableWindow->breakButtonClicked(); }, Qt::DirectConnection);
   };
   */
}
} // namespace pkt::ui::qtwidgets
