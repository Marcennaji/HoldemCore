#include "GuiBridgeWidgets.h"
#include "core/engine/GameEvents.h"
#include "core/engine/model/PlayerAction.h"
#include "core/session/Session.h"
#include "core/player/strategy/HumanStrategy.h"
#include "ui/qtwidgets/windows/PokerTableWindow.h"

#include <QString>
#include <QDebug>
#include <QCoreApplication>
#include <QEventLoop>

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
    connect(myTableWindow, &PokerTableWindow::nextHandRequested, this, &GuiBridgeWidgets::onNextHandRequested);
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
    
    events.onHumanStrategyWaiting = [this](pkt::core::player::HumanStrategy* strategy) {
        myCurrentHumanStrategy = strategy;
    };
    
    events.onProcessEvents = []() {
        // Qt-specific implementation: Process Qt events to keep GUI responsive
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
    };
}

// User action handlers (UI → Game Engine)
void GuiBridgeWidgets::onPlayerFold()
{
    qDebug() << "Player clicked Fold";
    PlayerAction action{0, ActionType::Fold, 0}; // Human player is always ID 0
    if (myCurrentHumanStrategy) {
        myCurrentHumanStrategy->setPlayerAction(action);
        myCurrentHumanStrategy = nullptr; // Clear after use
    }
}

void GuiBridgeWidgets::onPlayerCall()
{
    qDebug() << "Player clicked Call";
    PlayerAction action{0, ActionType::Call, 0}; 
    if (myCurrentHumanStrategy) {
        myCurrentHumanStrategy->setPlayerAction(action);
        myCurrentHumanStrategy = nullptr; // Clear after use
    }
}

void GuiBridgeWidgets::onPlayerCheck()
{
    qDebug() << "Player clicked Check";
    PlayerAction action{0, ActionType::Check, 0}; 
    if (myCurrentHumanStrategy) {
        myCurrentHumanStrategy->setPlayerAction(action);
        myCurrentHumanStrategy = nullptr; // Clear after use
    }
}

void GuiBridgeWidgets::onPlayerBet(int amount)
{
    qDebug() << "Player clicked Bet with amount:" << amount;
    PlayerAction action{0, ActionType::Bet, amount}; 
    if (myCurrentHumanStrategy) {
        myCurrentHumanStrategy->setPlayerAction(action);
        myCurrentHumanStrategy = nullptr; // Clear after use
    }
}

void GuiBridgeWidgets::onPlayerRaise(int amount)
{
    qDebug() << "Player clicked Raise with amount:" << amount;
    PlayerAction action{0, ActionType::Raise, amount}; 
    if (myCurrentHumanStrategy) {
        myCurrentHumanStrategy->setPlayerAction(action);
        myCurrentHumanStrategy = nullptr; // Clear after use
    }
}

void GuiBridgeWidgets::onPlayerAllIn()
{
    qDebug() << "Player clicked All-In";
    PlayerAction action{0, ActionType::Allin, 0}; 
    if (myCurrentHumanStrategy) {
        myCurrentHumanStrategy->setPlayerAction(action);
        myCurrentHumanStrategy = nullptr; // Clear after use
    }
}

void GuiBridgeWidgets::onNextHandRequested()
{
    qDebug() << "Next hand requested by user";
    
    // Update UI status
    myTableWindow->updatePlayerStatus(-1, "Starting next hand...");
    myTableWindow->updateGamePhase(pkt::core::GameState::None);
    
    // Request the session to start a new hand
    mySession->startNewHand();
}

// Game event handlers (Game Engine → UI)
void GuiBridgeWidgets::handleGameInitialized(int gameSpeed)
{
    qDebug() << "Game initialized with speed:" << gameSpeed;
    
    // Reset the UI for a new game
    myTableWindow->updatePlayerStatus(-1, "Game Started");
    myTableWindow->refreshPot(0); // Reset pot to 0
    myTableWindow->updateGamePhase(pkt::core::GameState::Preflop);
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
    
    // Show the Next Hand button when hand is completed
    myTableWindow->onHandCompleted();
}

void GuiBridgeWidgets::handlePlayerChipsUpdated(unsigned playerId, int newChips)
{
    qDebug() << "Player" << playerId << "chips updated to:" << newChips;
    // The refreshPlayer method will be called separately with full player data
}

void GuiBridgeWidgets::handleBettingRoundStarted(pkt::core::GameState gameState)
{
    qDebug() << "Betting round started. Game state:" << static_cast<int>(gameState);
    
    // Update the game phase display
    myTableWindow->updateGamePhase(gameState);
    
    // Clear previous round status messages
    myTableWindow->updatePlayerStatus(-1, "New betting round started");
    
    // Disable input until it's specifically requested
    myTableWindow->enablePlayerInput(false);
}

void GuiBridgeWidgets::handlePotUpdated(int newPotAmount)
{
    qDebug() << "Pot updated to:" << newPotAmount;
    myTableWindow->refreshPot(newPotAmount);
}

void GuiBridgeWidgets::handlePlayerActed(pkt::core::PlayerAction action)
{
    qDebug() << "Player" << action.playerId << "acted:" << static_cast<int>(action.type) << "Amount:" << action.amount;
    
    QString actionText = QString("Player %1: %2").arg(action.playerId).arg(QString::fromUtf8(actionTypeToString(action.type)));
    if (action.amount > 0) {
        actionText += QString(" ($%1)").arg(action.amount);
    }
    
    myTableWindow->updatePlayerStatus(action.playerId, actionText);
}

void GuiBridgeWidgets::handleAwaitingHumanInput(unsigned playerId, std::vector<pkt::core::ActionType> validActions)
{
    qDebug() << "Awaiting input from player:" << playerId << "Valid actions count:" << validActions.size();
    
    // Set available actions first
    myTableWindow->setAvailableActions(validActions);
    
    // Enable UI for human player input
    myTableWindow->enablePlayerInput(true);
    
    // Create a helpful status message showing available actions
    QString actionText = QString("Player %1's turn - Available: ").arg(playerId);
    QStringList actionStrings;
    for (const auto& action : validActions) {
        actionStrings << QString::fromUtf8(actionTypeToString(action));
    }
    actionText += actionStrings.join(", ");
    
    myTableWindow->updatePlayerStatus(playerId, actionText);
}

void GuiBridgeWidgets::handleHoleCardsDealt(unsigned playerId, pkt::core::HoleCards holeCards)
{
    qDebug() << "Hole cards dealt to player:" << playerId;
    
    // Show cards for human player (assuming player 0 is human for now)
    // TODO: Determine which player is the human player from session/game config
    if (playerId == 0) {
        myTableWindow->showHoleCards(playerId, holeCards);
        myTableWindow->updatePlayerStatus(playerId, "Your cards dealt - Good luck!");
    } else {
        myTableWindow->updatePlayerStatus(playerId, "Cards dealt");
    }
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
