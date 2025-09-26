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
#include <QElapsedTimer>
#include <QThread>
#include <QTimer>

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

    events.onShowdownRevealOrder = [this](std::vector<unsigned> revealOrder) {
        this->handleShowdownRevealOrder(revealOrder);
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
    
    // Reset the UI immediately for the new hand
    myTableWindow->resetForNewHand();
    
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
    m_gameSpeed = gameSpeed;
    
    if (myTableWindow) {
        m_humanChips = myTableWindow->getStartMoney();
        qDebug() << "Reset human chips tracking to start money:" << m_humanChips;
    } else {
        m_humanChips = -1; // Fallback if table window not available
    }
    
    // Reset the UI for a new hand
    myTableWindow->resetForNewHand();
    
    // Reset the UI for a new game
    myTableWindow->updatePlayerStatus(-1, "Game Started");
    myTableWindow->refreshPot(0); // Reset pot to 0
    myTableWindow->updateGamePhase(pkt::core::GameState::Preflop);
    myTableWindow->enablePlayerInput(false); // Wait for cards to be dealt
}

void GuiBridgeWidgets::handleShowdownRevealOrder(const std::vector<unsigned>& revealOrder)
{
    if (!myTableWindow) return;
    // Reveal each player's cached hole cards in the provided order
    myTableWindow->revealShowdownOrder(revealOrder);
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
    // Also show winner badge(s) over the winner area(s)
    myTableWindow->showWinners(winnerIds, totalPot);
    myTableWindow->enablePlayerInput(false);
    
    // Show the Next Hand button when hand is completed
    myTableWindow->onHandCompleted();
}

void GuiBridgeWidgets::handlePlayerChipsUpdated(unsigned playerId, int newChips)
{
    qDebug() << "Player" << playerId << "chips updated to:" << newChips;
    // Update the player's displayed stack immediately
    if (myTableWindow) {
        myTableWindow->updatePlayerCash(playerId, newChips);
    }

    // Track human chips and enforce auto-fold at 0
    if (playerId == 0) {
        m_humanChips = newChips;
        if (m_humanChips <= 0) {
            autoFoldHumanIfBroke();
        }
    }
}

void GuiBridgeWidgets::handleBettingRoundStarted(pkt::core::GameState gameState)
{
    qDebug() << "Betting round started. Game state: " << gameStateToString(gameState);
    
    // Update the game phase display
    myTableWindow->updateGamePhase(gameState);
    // Clear previous players' action labels so each round shows only current actions
    myTableWindow->clearActionLabelsForNewRound();
    
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
    qDebug() << "Player" << action.playerId << "acted:" << actionTypeToString(action.type) << ", amount:" << action.amount;
    
    myTableWindow->showPlayerAction(action.playerId, action.type, action.amount);

    // Maintain dealer indicator: infer dealer from SB post event once per hand
    if (action.type == ActionType::PostSmallBlind) {
        myTableWindow->setDealerFromSmallBlind(static_cast<int>(action.playerId));
    }

    // Insert a small, responsive delay between bot actions based on game speed.
    // Human actions (playerId == 0) should not be artificially delayed.
    if (action.playerId != 0) {
        const int delayMs = computeDelayMsForBots();
        if (delayMs > 0) {
            QElapsedTimer timer;
            timer.start();
            while (timer.elapsed() < delayMs) {
                QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
                QThread::msleep(10);
            }
        }
    }
}

void GuiBridgeWidgets::handleAwaitingHumanInput(unsigned playerId, std::vector<pkt::core::ActionType> validActions)
{
    qDebug() << "Awaiting input from player:" << playerId << "Valid actions count:" << validActions.size();
    
    // Set available actions first
    myTableWindow->setAvailableActions(validActions);
    
    // Enable UI for human player input
    // If human has 0 chips, auto-fold and do not enable input
    if (playerId == 0 && m_humanChips == 0) {
        autoFoldHumanIfBroke();
        return;
    }
    myTableWindow->enablePlayerInput(true);
    
    // Clear human's last action label just before their turn to avoid confusion
    if (playerId == 0) {
        myTableWindow->clearPlayerActionLabel(0);
    }

    // Structured turn update
    myTableWindow->showPlayerTurn(static_cast<int>(playerId));
}

void GuiBridgeWidgets::autoFoldHumanIfBroke()
{
    if (m_humanChips != 0) return; // only trigger at exactly 0
    // Disable input and submit a Fold action automatically
    if (myTableWindow) myTableWindow->enablePlayerInput(false);
    PlayerAction action{0, ActionType::Fold, 0};
    if (myCurrentHumanStrategy) {
        myCurrentHumanStrategy->setPlayerAction(action);
        myCurrentHumanStrategy = nullptr;
    }
}

void GuiBridgeWidgets::handleHoleCardsDealt(unsigned playerId, pkt::core::HoleCards holeCards)
{
    qDebug() << "Hole cards dealt to player " << playerId << "are" << holeCards.toString();
    
    // Show cards for human player (assuming player 0 is human for now)
    // TODO: Determine which player is the human player from session/game config
    if (playerId == 0) {
        myTableWindow->showHoleCards(playerId, holeCards);
    }
    // Cache everyone’s hole cards for potential showdown reveal
    myTableWindow->cacheHoleCards(static_cast<int>(playerId), holeCards);
}

void GuiBridgeWidgets::handleBoardCardsDealt(pkt::core::BoardCards boardCards)
{
    qDebug() << "Board cards dealt are: " << boardCards.toString();
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

int GuiBridgeWidgets::computeDelayMsForBots() const
{
    // Normalize m_gameSpeed to a reasonable range [1..10] if out of bounds
    int speed = m_gameSpeed <= 0 ? 1 : m_gameSpeed;
    if (speed > 50) speed = 50; // clamp to avoid zero or negative delays

    // Higher gameSpeed => faster pacing (shorter delay)
    // Map speed in [1..10] to delay in [2000..100] ms inversely (clamped)
    const int minDelay = 10;  
    const int maxDelay = 2000; 

    // If speed is large (e.g., 10), delay should be near minDelay.
    // Use a simple inverse proportion: delay = maxDelay - (speed-1) * step
    const int maxSpeedForScale = 10;
    int s = speed;
    if (s > maxSpeedForScale) s = maxSpeedForScale;
    const int step = (maxDelay - minDelay) / (maxSpeedForScale - 1); // divide range into 9 steps
    int delay = maxDelay - (s - 1) * step;
    if (delay < minDelay) delay = minDelay;
    if (delay > maxDelay) delay = maxDelay;
    return delay;
}

} // namespace pkt::ui::qtwidgets
