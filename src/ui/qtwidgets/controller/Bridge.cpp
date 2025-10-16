#include "Bridge.h"
#include "core/engine/GameEvents.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/Player.h"
#include "core/player/strategy/HumanStrategy.h"
#include "core/session/Session.h"
#include "ui/qtwidgets/dto/PlayerDisplayInfo.h"
#include "ui/qtwidgets/windows/PokerTableWindow.h"

#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QString>
#include <QThread>
#include <QTimer>

using namespace pkt::core;

namespace pkt::ui::qtwidgets::controller
{

Bridge::Bridge(pkt::core::Session* session, pkt::ui::qtwidgets::PokerTableWindow* pokerTableWindow, QObject* parent)
    : QObject(parent), m_session(session), m_tableWindow(pokerTableWindow)
{
    connectSignalsFromUi();
}

void Bridge::connectSignalsFromUi()
{
    // Connect UI signals to our slots for handling user actions
    connect(m_tableWindow, &pkt::ui::qtwidgets::PokerTableWindow::foldClicked, this, &Bridge::onPlayerFold);
    connect(m_tableWindow, &pkt::ui::qtwidgets::PokerTableWindow::callClicked, this, &Bridge::onPlayerCall);
    connect(m_tableWindow, &pkt::ui::qtwidgets::PokerTableWindow::checkClicked, this, &Bridge::onPlayerCheck);
    connect(m_tableWindow, &pkt::ui::qtwidgets::PokerTableWindow::betClicked, this, &Bridge::onPlayerBet);
    connect(m_tableWindow, &pkt::ui::qtwidgets::PokerTableWindow::raiseClicked, this, &Bridge::onPlayerRaise);
    connect(m_tableWindow, &pkt::ui::qtwidgets::PokerTableWindow::allInClicked, this, &Bridge::onPlayerAllIn);
    connect(m_tableWindow, &pkt::ui::qtwidgets::PokerTableWindow::nextHandRequested, this,
            &Bridge::onNextHandRequested);
}

void Bridge::connectEventsToUi(pkt::core::GameEvents& events)
{
    // Connect game engine events to UI update methods using lambdas to capture 'this'

    events.onPlayersInitialized = [this](const pkt::core::player::PlayerList& players)
    { this->handlePlayersInitialized(players); };

    events.onGameInitialized = [this](int gameSpeed) { this->handleGameInitialized(gameSpeed); };

    events.onHandCompleted = [this](std::list<unsigned> winnerIds, int totalPot)
    { this->handleHandCompleted(winnerIds, totalPot); };

    events.onPlayerChipsUpdated = [this](unsigned playerId, int newChips)
    { this->handlePlayerChipsUpdated(playerId, newChips); };

    events.onBettingRoundStarted = [this](GameState gameState) { this->handleBettingRoundStarted(gameState); };

    events.onPotUpdated = [this](int newPotAmount) { this->handlePotUpdated(newPotAmount); };

    events.onPlayerActed = [this](PlayerAction action) { this->handlePlayerActed(action); };

    events.onAwaitingHumanInput = [this](unsigned playerId, std::vector<ActionType> validActions)
    { this->handleAwaitingHumanInput(playerId, validActions); };

    events.onHoleCardsDealt = [this](unsigned playerId, HoleCards holeCards)
    { this->handleHoleCardsDealt(playerId, holeCards); };

    events.onBoardCardsDealt = [this](BoardCards boardCards) { this->handleBoardCardsDealt(boardCards); };

    events.onShowdownRevealOrder = [this](std::vector<unsigned> revealOrder)
    { this->handleShowdownRevealOrder(revealOrder); };

    events.onInvalidPlayerAction = [this](unsigned playerId, PlayerAction invalidAction, std::string reason)
    { this->handleInvalidPlayerAction(playerId, invalidAction, reason); };

    events.onEngineError = [this](std::string errorMessage) { this->handleEngineError(errorMessage); };

    events.onHumanStrategyWaiting = [this](pkt::core::player::HumanStrategy* strategy)
    { m_currentHumanStrategy = strategy; };

    events.onProcessEvents = []()
    {
        // Qt-specific implementation: Process Qt events to keep GUI responsive
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
    };
}

// User action handlers (UI → Game Engine)
void Bridge::onPlayerFold()
{
    qDebug() << "Player clicked Fold";
    PlayerAction action{0, ActionType::Fold, 0}; // Human player is always ID 0
    if (m_currentHumanStrategy)
    {
        m_currentHumanStrategy->setPlayerAction(action);
        m_currentHumanStrategy = nullptr; // Clear after use
    }
}

void Bridge::onPlayerCall()
{
    qDebug() << "Player clicked Call";
    PlayerAction action{0, ActionType::Call, 0};
    if (m_currentHumanStrategy)
    {
        m_currentHumanStrategy->setPlayerAction(action);
        m_currentHumanStrategy = nullptr; // Clear after use
    }
}

void Bridge::onPlayerCheck()
{
    qDebug() << "Player clicked Check";
    PlayerAction action{0, ActionType::Check, 0};
    if (m_currentHumanStrategy)
    {
        m_currentHumanStrategy->setPlayerAction(action);
        m_currentHumanStrategy = nullptr; // Clear after use
    }
}

void Bridge::onPlayerBet(int amount)
{
    qDebug() << "Player clicked Bet with amount:" << amount;
    PlayerAction action{0, ActionType::Bet, amount};
    if (m_currentHumanStrategy)
    {
        m_currentHumanStrategy->setPlayerAction(action);
        m_currentHumanStrategy = nullptr; // Clear after use
    }
}

void Bridge::onPlayerRaise(int amount)
{
    qDebug() << "Player clicked Raise with amount:" << amount;
    PlayerAction action{0, ActionType::Raise, amount};
    if (m_currentHumanStrategy)
    {
        m_currentHumanStrategy->setPlayerAction(action);
        m_currentHumanStrategy = nullptr; // Clear after use
    }
}

void Bridge::onPlayerAllIn()
{
    qDebug() << "Player clicked All-In";
    PlayerAction action{0, ActionType::Allin, 0};
    if (m_currentHumanStrategy)
    {
        m_currentHumanStrategy->setPlayerAction(action);
        m_currentHumanStrategy = nullptr; // Clear after use
    }
}

void Bridge::onNextHandRequested()
{
    qDebug() << "Next hand requested by user";

    // Reset the UI immediately for the new hand
    m_tableWindow->resetForNewHand();

    // Update UI status
    m_tableWindow->updateGamePhase(pkt::core::GameState::None);

    // Request the session to start a new hand
    m_session->startNewHand();
}

// Game event handlers (Game Engine → UI)
void Bridge::handlePlayersInitialized(const pkt::core::player::PlayerList& players)
{
    qDebug() << "Players initialized, updating UI with player information";

    if (!m_tableWindow || !players)
        return;

    int seat = 0;
    for (const auto& player : *players)
    {
        if (player)
        {
            // Convert Player to DTO and refresh UI
            pkt::ui::qtwidgets::PlayerDisplayInfo displayInfo = createPlayerDisplayInfo(*player);
            m_tableWindow->refreshPlayer(seat, displayInfo);
            qDebug() << "  Seat" << seat << ":" << QString::fromStdString(displayInfo.playerName) << "("
                     << QString::fromStdString(displayInfo.strategyName) << ")"
                     << "chips:" << displayInfo.chips;
        }
        ++seat;
    }
}

void Bridge::handleGameInitialized(int gameSpeed)
{
    qDebug() << "Game initialized with speed:" << gameSpeed;
    m_gameSpeed = gameSpeed;

    if (m_tableWindow)
    {
        m_humanChips = m_tableWindow->getStartMoney();
        qDebug() << "Reset human chips tracking to start money:" << m_humanChips;
    }
    else
    {
        m_humanChips = -1; // Fallback if table window not available
    }

    // Reset the UI for a new hand
    m_tableWindow->resetForNewHand();

    // Reset the UI for a new game
    m_tableWindow->refreshPot(0); // Reset pot to 0
    m_tableWindow->updateGamePhase(pkt::core::GameState::Preflop);
    m_tableWindow->enablePlayerInput(false); // Wait for cards to be dealt
}

void Bridge::handleShowdownRevealOrder(const std::vector<unsigned>& revealOrder)
{
    if (!m_tableWindow)
        return;
    // Reveal each player's cached hole cards in the provided order
    m_tableWindow->revealShowdownOrder(revealOrder);
}

void Bridge::handleHandCompleted(std::list<unsigned> winnerIds, int totalPot)
{
    qDebug() << "Hand completed. Total pot:" << totalPot;

    QString winnerText = "Winners: ";
    for (auto winnerId : winnerIds)
    {
        winnerText += QString::number(winnerId) + " ";
    }
    winnerText += QString("- Pot: $%1").arg(totalPot);

    m_tableWindow->showWinners(winnerIds, totalPot);
    m_tableWindow->enablePlayerInput(false);

    // IMPORTANT: Use QTimer to defer the call to onHandCompleted
    // This allows the game engine to complete its cleanup (pot distribution, state transitions)
    // before the UI processes the hand completion and potentially starts a new hand.
    // Without this delay, we get a race condition where the new hand starts before
    // the old hand is fully cleaned up, causing access violations.
    QTimer::singleShot(50, this,
                       [this]()
                       {
                           if (m_tableWindow)
                           {
                               m_tableWindow->onHandCompleted();
                           }
                       });
}

void Bridge::handlePlayerChipsUpdated(unsigned playerId, int newChips)
{
    qDebug() << "Player" << playerId << "chips updated to:" << newChips;
    // Update the player's displayed stack immediately
    if (m_tableWindow)
    {
        m_tableWindow->updatePlayerCash(playerId, newChips);
    }

    // Track human chips and enforce auto-fold at 0
    if (playerId == 0)
    {
        m_humanChips = newChips;
        if (m_humanChips <= 0)
        {
            autoFoldHumanIfBroke();
        }
    }
}

void Bridge::handleBettingRoundStarted(pkt::core::GameState gameState)
{
    qDebug() << "Betting round started. Game state: " << gameStateToString(gameState);

    // Update the game phase display
    m_tableWindow->updateGamePhase(gameState);
    // Clear previous players' action labels so each round shows only current actions
    m_tableWindow->clearActionLabelsForNewRound();

    // Disable input until it's specifically requested
    m_tableWindow->enablePlayerInput(false);
}

void Bridge::handlePotUpdated(int newPotAmount)
{
    qDebug() << "Pot updated to:" << newPotAmount;
    m_tableWindow->refreshPot(newPotAmount);
}

void Bridge::handlePlayerActed(pkt::core::PlayerAction action)
{
    qDebug() << "Player" << action.playerId << "acted:" << actionTypeToString(action.type)
             << ", amount:" << action.amount;

    m_tableWindow->showPlayerAction(action.playerId, action.type, action.amount);

    // Maintain dealer indicator: infer dealer from SB post event once per hand
    if (action.type == ActionType::PostSmallBlind)
    {
        m_tableWindow->setDealerFromSmallBlind(static_cast<int>(action.playerId));
    }

    // Insert a small, responsive delay between bot actions based on game speed.
    // Human actions (playerId == 0) should not be artificially delayed.
    if (action.playerId != 0)
    {
        const int delayMs = computeDelayMsForBots();
        if (delayMs > 0)
        {
            QElapsedTimer timer;
            timer.start();
            while (timer.elapsed() < delayMs)
            {
                QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
                QThread::msleep(10);
            }
        }
    }
}

void Bridge::handleAwaitingHumanInput(unsigned playerId, std::vector<pkt::core::ActionType> validActions)
{
    qDebug() << "Awaiting input from player:" << playerId << "Valid actions count:" << validActions.size();

    // Set available actions first
    m_tableWindow->setAvailableActions(validActions);

    // Enable UI for human player input
    // If human has 0 chips, auto-fold and do not enable input
    if (playerId == 0 && m_humanChips == 0)
    {
        autoFoldHumanIfBroke();
        return;
    }
    m_tableWindow->enablePlayerInput(true);

    // Clear human's last action label just before their turn to avoid confusion
    if (playerId == 0)
    {
        m_tableWindow->clearPlayerActionLabel(0);
    }

    // Structured turn update
    m_tableWindow->showPlayerTurn(static_cast<int>(playerId));
}

void Bridge::autoFoldHumanIfBroke()
{
    if (m_humanChips != 0)
        return; // only trigger at exactly 0
    // Disable input and submit a Fold action automatically
    if (m_tableWindow)
        m_tableWindow->enablePlayerInput(false);
    PlayerAction action{0, ActionType::Fold, 0};
    if (m_currentHumanStrategy)
    {
        m_currentHumanStrategy->setPlayerAction(action);
        m_currentHumanStrategy = nullptr;
    }
}

void Bridge::handleHoleCardsDealt(unsigned playerId, pkt::core::HoleCards holeCards)
{
    qDebug() << "Hole cards dealt to player " << playerId << "are" << holeCards.toString();

    // Show cards for human player (assuming player 0 is human for now)
    // TODO: Determine which player is the human player from session/game config
    if (playerId == 0)
    {
        m_tableWindow->showHoleCards(playerId, holeCards);
    }
    // Cache everyone’s hole cards for potential showdown reveal
    m_tableWindow->cacheHoleCards(static_cast<int>(playerId), holeCards);
}

void Bridge::handleBoardCardsDealt(pkt::core::BoardCards boardCards)
{
    qDebug() << "Board cards dealt are: " << boardCards.toString();
    m_tableWindow->showBoardCards(boardCards);
}

void Bridge::handleInvalidPlayerAction(unsigned playerId, pkt::core::PlayerAction invalidAction, std::string reason)
{
    qDebug() << "Invalid action from player:" << playerId << "Reason:" << QString::fromStdString(reason);

    QString errorMsg = QString("Invalid action: %1").arg(QString::fromStdString(reason));
    m_tableWindow->showErrorMessage(errorMsg);
}

void Bridge::handleEngineError(std::string errorMessage)
{
    qDebug() << "Engine error:" << QString::fromStdString(errorMessage);
    m_tableWindow->showErrorMessage(QString::fromStdString(errorMessage));
}

int Bridge::computeDelayMsForBots() const
{
    // Normalize m_gameSpeed to a reasonable range [1..10] if out of bounds
    int speed = m_gameSpeed <= 0 ? 1 : m_gameSpeed;
    if (speed > 50)
        speed = 50; // clamp to avoid zero or negative delays

    // Higher gameSpeed => faster pacing (shorter delay)
    // Map speed in [1..10] to delay in [2000..100] ms inversely (clamped)
    const int minDelay = 10;
    const int maxDelay = 2000;

    // If speed is large (e.g., 10), delay should be near minDelay.
    // Use a simple inverse proportion: delay = maxDelay - (speed-1) * step
    const int maxSpeedForScale = 10;
    int s = speed;
    if (s > maxSpeedForScale)
        s = maxSpeedForScale;
    const int step = (maxDelay - minDelay) / (maxSpeedForScale - 1); // divide range into 9 steps
    int delay = maxDelay - (s - 1) * step;
    if (delay < minDelay)
        delay = minDelay;
    if (delay > maxDelay)
        delay = maxDelay;
    return delay;
}

pkt::ui::qtwidgets::PlayerDisplayInfo Bridge::createPlayerDisplayInfo(const pkt::core::player::Player& player) const
{
    return pkt::ui::qtwidgets::PlayerDisplayInfo(player.getId(), player.getName(), player.getStrategyName(),
                                                 player.getCash());
}

} // namespace pkt::ui::qtwidgets::controller
