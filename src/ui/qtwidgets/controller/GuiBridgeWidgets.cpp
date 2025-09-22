#include "GuiBridgeWidgets.h"
#include "core/engine/GameEvents.h"
#include "core/session/Session.h"
#include "ui/qtwidgets/poker_ui/PokerTableWindow.h"

#include <QString>
#include <QDebug>

using namespace pkt::core;

namespace pkt::ui::qtwidgets
{

GuiBridgeWidgets::GuiBridgeWidgets(pkt::core::Session* session, PokerTableWindow* pokerTableWindow, QObject* parent)
    : QObject(parent), mySession(session), myTableWindow(pokerTableWindow)
{
    connectSignalsFromUi();
}

void GuiBridgeWidgets::connectSignalsFromUi()
{
    // Connect UI signals to our slots for handling user actions
    connect(myTableWindow, &PokerTableWindow::foldClicked, this, &GuiBridgeWidgets::onPlayerFold);
    connect(myTableWindow, &PokerTableWindow::callClicked, this, &GuiBridgeWidgets::onPlayerCall);
    connect(myTableWindow, &PokerTableWindow::checkClicked, this, &GuiBridgeWidgets::onPlayerCheck);
    connect(myTableWindow, &PokerTableWindow::betClicked, this, &GuiBridgeWidgets::onPlayerBet);
    connect(myTableWindow, &PokerTableWindow::raiseClicked, this, &GuiBridgeWidgets::onPlayerRaise);
    connect(myTableWindow, &PokerTableWindow::allInClicked, this, &GuiBridgeWidgets::onPlayerAllIn);
}

void GuiBridgeWidgets::connectEventsToUi(pkt::core::GameEvents& events)
{
    // Connect game engine events to UI update methods using lambdas to capture 'this'
    
    events.onGameInitialized = [this](int gameSpeed) {
        this->handleGameInitialized(gameSpeed);
    };
    
    events.onHandCompleted = [this](std::list<unsigned> winnerIds, int totalPot) {
        this->handleHandCompleted(winnerIds, totalPot);
    };
    
    events.onPlayerChipsUpdated = [this](unsigned playerId, int newChips) {
        this->handlePlayerChipsUpdated(playerId, newChips);
    };
    
    events.onBettingRoundStarted = [this](GameState gameState) {
        this->handleBettingRoundStarted(gameState);
    };
    
    events.onPotUpdated = [this](int newPotAmount) {
        this->handlePotUpdated(newPotAmount);
    };
    
    events.onPlayerActed = [this](PlayerAction action) {
        this->handlePlayerActed(action);
    };
    
    events.onAwaitingHumanInput = [this](unsigned playerId, std::vector<ActionType> validActions) {
        this->handleAwaitingHumanInput(playerId, validActions);
    };
    
    events.onHoleCardsDealt = [this](unsigned playerId, HoleCards holeCards) {
        this->handleHoleCardsDealt(playerId, holeCards);
    };
    
    events.onBoardCardsDealt = [this](BoardCards boardCards) {
        this->handleBoardCardsDealt(boardCards);
    };
    
    events.onInvalidPlayerAction = [this](unsigned playerId, PlayerAction invalidAction, std::string reason) {
        this->handleInvalidPlayerAction(playerId, invalidAction, reason);
    };
    
    events.onEngineError = [this](std::string errorMessage) {
        this->handleEngineError(errorMessage);
    };
}

// User action handlers (UI → Game Engine)
void GuiBridgeWidgets::onPlayerFold()
{
    qDebug() << "Player clicked Fold";
    // TODO: Send fold action to game engine through session
    // PlayerAction action{playerId, ActionType::Fold, 0};
    // mySession->handlePlayerAction(action);
}

void GuiBridgeWidgets::onPlayerCall()
{
    qDebug() << "Player clicked Call";
    // TODO: Send call action to game engine
}

void GuiBridgeWidgets::onPlayerCheck()
{
    qDebug() << "Player clicked Check";
    // TODO: Send check action to game engine
}

void GuiBridgeWidgets::onPlayerBet(int amount)
{
    qDebug() << "Player clicked Bet with amount:" << amount;
    // TODO: Send bet action to game engine
}

void GuiBridgeWidgets::onPlayerRaise(int amount)
{
    qDebug() << "Player clicked Raise with amount:" << amount;
    // TODO: Send raise action to game engine
}

void GuiBridgeWidgets::onPlayerAllIn()
{
    qDebug() << "Player clicked All-In";
    // TODO: Send all-in action to game engine
}

// Game event handlers (Game Engine → UI)
void GuiBridgeWidgets::handleGameInitialized(int gameSpeed)
{
    qDebug() << "Game initialized with speed:" << gameSpeed;
    myTableWindow->updatePlayerStatus(-1, "Game Started");
    myTableWindow->enablePlayerInput(false); // Wait for cards to be dealt
}

void GuiBridgeWidgets::handleHandCompleted(std::list<unsigned> winnerIds, int totalPot)
{
    qDebug() << "Hand completed. Total pot:" << totalPot;
    
    QString winnerText = "Winners: ";
    for (auto winnerId : winnerIds) {
        winnerText += QString::number(winnerId) + " ";
    }
    winnerText += QString("- Pot: $%1").arg(totalPot);
    
    myTableWindow->updatePlayerStatus(-1, winnerText);
    myTableWindow->enablePlayerInput(false);
}

void GuiBridgeWidgets::handlePlayerChipsUpdated(unsigned playerId, int newChips)
{
    qDebug() << "Player" << playerId << "chips updated to:" << newChips;
    // The refreshPlayer method will be called separately with full player data
}

void GuiBridgeWidgets::handleBettingRoundStarted(pkt::core::GameState gameState)
{
    qDebug() << "Betting round started. Game state:" << static_cast<int>(gameState);
    myTableWindow->updateGamePhase(gameState);
}

void GuiBridgeWidgets::handlePotUpdated(int newPotAmount)
{
    qDebug() << "Pot updated to:" << newPotAmount;
    myTableWindow->refreshPot(newPotAmount);
}

void GuiBridgeWidgets::handlePlayerActed(pkt::core::PlayerAction action)
{
    qDebug() << "Player" << action.playerId << "acted:" << static_cast<int>(action.type) << "Amount:" << action.amount;
    
    QString actionText = QString("Player %1: %2").arg(action.playerId).arg(actionTypeToString(action.type));
    if (action.amount > 0) {
        actionText += QString(" ($%1)").arg(action.amount);
    }
    
    myTableWindow->updatePlayerStatus(action.playerId, actionText);
}

void GuiBridgeWidgets::handleAwaitingHumanInput(unsigned playerId, std::vector<pkt::core::ActionType> validActions)
{
    qDebug() << "Awaiting input from player:" << playerId << "Valid actions count:" << validActions.size();
    
    // Enable UI for human player input
    myTableWindow->setAvailableActions(validActions);
    myTableWindow->enablePlayerInput(true);
    
    QString actionText = QString("Player %1's turn").arg(playerId);
    myTableWindow->updatePlayerStatus(playerId, actionText);
}

void GuiBridgeWidgets::handleHoleCardsDealt(unsigned playerId, pkt::core::HoleCards holeCards)
{
    qDebug() << "Hole cards dealt to player:" << playerId;
    
    // Show cards for human player (assuming player 0 is human for now)
    // TODO: Determine which player is the human player
    if (playerId == 0) {
        myTableWindow->showHoleCards(playerId, holeCards);
    }
    
    QString statusText = "Cards dealt";
    myTableWindow->updatePlayerStatus(playerId, statusText);
}

void GuiBridgeWidgets::handleBoardCardsDealt(pkt::core::BoardCards boardCards)
{
    qDebug() << "Board cards dealt. Number of cards:" << boardCards.numCards;
    myTableWindow->showBoardCards(boardCards);
}

void GuiBridgeWidgets::handleInvalidPlayerAction(unsigned playerId, pkt::core::PlayerAction invalidAction, std::string reason)
{
    qDebug() << "Invalid action from player:" << playerId << "Reason:" << QString::fromStdString(reason);
    
    QString errorMsg = QString("Invalid action: %1").arg(QString::fromStdString(reason));
    myTableWindow->showErrorMessage(errorMsg);
}

void GuiBridgeWidgets::handleEngineError(std::string errorMessage)
{
    qDebug() << "Engine error:" << QString::fromStdString(errorMessage);
    myTableWindow->showErrorMessage(QString::fromStdString(errorMessage));
}

} // namespace pkt::ui::qtwidgets
