// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Bridge.h"
#include "TableViewModel.h"

#include <core/engine/GameEvents.h>
#include <core/engine/model/PlayerAction.h>
#include <core/player/Player.h>
#include <core/player/strategy/HumanStrategy.h>
#include <core/session/Session.h>

#include <QCoreApplication>
#include <QDebug>
#include <QEventLoop>
#include <QVariantMap>
#include <map>

using namespace pkt::core;

namespace pkt::ui::qtqml::controller
{

Bridge::Bridge(pkt::core::Session* session, TableViewModel* viewModel, QObject* parent)
    : QObject(parent), m_session(session), m_viewModel(viewModel)
{
}

void Bridge::connectEventsToViewModel(pkt::core::GameEvents& events)
{
    // Connect game engine events to view model update methods

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
        // Qt-specific: Process Qt events to keep GUI responsive
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
    };
}

// User action handlers (QML UI → Game Engine)

void Bridge::onPlayerFold()
{
    qDebug() << "QML: Player clicked Fold";
    PlayerAction action{0, ActionType::Fold, 0}; // Human player is always ID 0
    if (m_currentHumanStrategy)
    {
        m_currentHumanStrategy->setPlayerAction(action);
        m_currentHumanStrategy = nullptr;
    }
}

void Bridge::onPlayerCall()
{
    qDebug() << "QML: Player clicked Call";
    PlayerAction action{0, ActionType::Call, 0};
    if (m_currentHumanStrategy)
    {
        m_currentHumanStrategy->setPlayerAction(action);
        m_currentHumanStrategy = nullptr;
    }
}

void Bridge::onPlayerCheck()
{
    qDebug() << "QML: Player clicked Check";
    PlayerAction action{0, ActionType::Check, 0};
    if (m_currentHumanStrategy)
    {
        m_currentHumanStrategy->setPlayerAction(action);
        m_currentHumanStrategy = nullptr;
    }
}

void Bridge::onPlayerBet(int amount)
{
    qDebug() << "QML: Player clicked Bet with amount:" << amount;
    PlayerAction action{0, ActionType::Bet, amount};
    if (m_currentHumanStrategy)
    {
        m_currentHumanStrategy->setPlayerAction(action);
        m_currentHumanStrategy = nullptr;
    }
}

void Bridge::onPlayerRaise(int amount)
{
    qDebug() << "QML: Player clicked Raise with amount:" << amount;
    PlayerAction action{0, ActionType::Raise, amount};
    if (m_currentHumanStrategy)
    {
        m_currentHumanStrategy->setPlayerAction(action);
        m_currentHumanStrategy = nullptr;
    }
}

void Bridge::onPlayerAllIn()
{
    qDebug() << "QML: Player clicked All-In";
    PlayerAction action{0, ActionType::Allin, 0};
    if (m_currentHumanStrategy)
    {
        m_currentHumanStrategy->setPlayerAction(action);
        m_currentHumanStrategy = nullptr;
    }
}

void Bridge::onNextHandRequested()
{
    qDebug() << "QML: Next hand requested by user - resetting UI for new hand";
    m_viewModel->setHandResult("");
    m_viewModel->setWinners(QVariantList());

    // Clear board cards explicitly (fix bug where 2nd hand showed all community cards at preflop)
    m_viewModel->setBoardCards(QVariantList());

    // Clear cached hole cards for new hand
    m_cachedHoleCards.clear();

    // Reset all players for new hand
    for (int i = 0; i < m_viewModel->players().size(); ++i)
    {
        QVariantMap player = m_viewModel->players()[i].toMap();
        int playerId = player["id"].toInt();

        // Reset folded state
        m_viewModel->updatePlayerFolded(playerId, false);

        // Hide cards (show card backs)
        m_viewModel->updatePlayerCards(playerId, "??", "??");

        // Clear last action
        m_viewModel->updatePlayerAction(playerId, "", 0);
    }

    m_session->startNewHand();
}

// Game event handlers (Game Engine → View Model)

void Bridge::handlePlayersInitialized(const pkt::core::player::PlayerList& players)
{
    qDebug() << "QML: Players initialized, updating view model";

    if (!players)
        return;

    QVariantList playerList;
    int seat = 0;

    for (const auto& player : *players)
    {
        if (player)
        {
            QVariantMap playerData;
            playerData["id"] = player->getId();
            playerData["name"] = QString::fromStdString(player->getName());
            playerData["chips"] = player->getCash();
            playerData["seat"] = seat;
            playerData["isActive"] = true; // All initialized players are active
            playerData["hasCards"] = false;
            playerData["card1"] = "";
            playerData["card2"] = "";
            playerData["lastAction"] = "";
            playerData["lastBet"] = 0;

            // Check if this is the human player (ID 0)
            if (player->getId() == 0)
            {
                m_viewModel->setHumanPlayerId(0);
            }

            playerList.append(playerData);
        }
        ++seat;
    }

    // Store player count for dealer position calculation
    m_playerCount = static_cast<int>(players->size());

    m_viewModel->setPlayers(playerList);
}

void Bridge::handleGameInitialized(int gameSpeed)
{
    qDebug() << "QML: Game initialized with speed:" << gameSpeed;
    m_gameSpeed = gameSpeed;

    m_viewModel->setPotAmount(0);
    m_viewModel->setGameStateText("Game Starting");
    m_viewModel->setBoardCards(QVariantList());
    m_viewModel->setAwaitingHumanInput(false);
}

void Bridge::handleHandCompleted(std::list<unsigned> winnerIds, int totalPot)
{
    qDebug() << "QML: Hand completed. Total pot:" << totalPot;

    QString resultText = "Winners: ";
    QVariantList winners;

    for (auto winnerId : winnerIds)
    {
        resultText += QString::number(winnerId) + " ";
        winners.append(static_cast<int>(winnerId));
    }
    resultText += QString(" - Pot: $%1").arg(totalPot);

    m_viewModel->setHandResult(resultText);
    m_viewModel->setWinners(winners);
    m_viewModel->setAwaitingHumanInput(false);
}

void Bridge::handleShowdownRevealOrder(const std::vector<unsigned>& revealOrder)
{
    qDebug() << "QML: Showdown reveal order received. Revealing" << revealOrder.size() << "players";

    // Reveal cards for all non-folded players in showdown order
    for (unsigned playerId : revealOrder)
    {
        // Check if we have cached hole cards for this player
        auto it = m_cachedHoleCards.find(playerId);
        if (it != m_cachedHoleCards.end())
        {
            const auto& holeCards = it->second;
            QString card1 = cardToString(holeCards.card1);
            QString card2 = cardToString(holeCards.card2);

            qDebug() << "QML: Revealing player" << playerId << "cards:" << card1 << card2;
            m_viewModel->updatePlayerCards(playerId, card1, card2);
        }
    }
}

void Bridge::handlePlayerChipsUpdated(unsigned playerId, int newChips)
{
    qDebug() << "QML: Player" << playerId << "chips updated to:" << newChips;
    m_viewModel->updatePlayerChips(playerId, newChips);
}

void Bridge::handleBettingRoundStarted(pkt::core::GameState gameState)
{
    QString stateText = gameStateToString(gameState);
    qDebug() << "QML: Betting round started:" << stateText;

    m_viewModel->setGameStateText(stateText);
    m_viewModel->setAwaitingHumanInput(false);

    // Clear action labels from previous betting round (like Qt Widgets clearActionLabelsForNewRound)
    // This ensures each round shows only current actions, not previous round's actions
    for (int i = 0; i < m_viewModel->players().size(); ++i)
    {
        QVariantMap player = m_viewModel->players()[i].toMap();
        int playerId = player["id"].toInt();
        m_viewModel->updatePlayerAction(playerId, "", 0);
    }
}

void Bridge::handlePotUpdated(int newPotAmount)
{
    qDebug() << "QML: Pot updated to:" << newPotAmount;
    m_viewModel->setPotAmount(newPotAmount);
}

void Bridge::handlePlayerActed(pkt::core::PlayerAction action)
{
    QString actionText = actionTypeToString(action.type);
    qDebug() << "QML: Player" << action.playerId << "acted:" << actionText << ", amount:" << action.amount;

    m_viewModel->updatePlayerAction(action.playerId, actionText, action.amount);
    m_viewModel->setCurrentPlayerId(action.playerId);

    // Mark player as folded for visual effects
    if (action.type == pkt::core::ActionType::Fold)
    {
        m_viewModel->updatePlayerFolded(action.playerId, true);
    }

    // Track dealer position from small blind (like Qt Widgets)
    if (action.type == pkt::core::ActionType::PostSmallBlind)
    {
        setDealerFromSmallBlind(action.playerId);
    }
}

void Bridge::handleAwaitingHumanInput(unsigned playerId, std::vector<pkt::core::ActionType> validActions)
{
    qDebug() << "QML: Awaiting human input for player" << playerId;

    QStringList actionStrings;
    for (auto action : validActions)
    {
        actionStrings.append(actionTypeToString(action));
    }

    m_viewModel->setValidActions(actionStrings);
    m_viewModel->setAwaitingHumanInput(true);
    m_viewModel->setCurrentPlayerId(playerId);

    // TODO: Set min/max bet based on game rules
    // This will require additional context from the engine
    m_viewModel->setMinBet(0);
    m_viewModel->setMaxBet(1000);
}

void Bridge::handleHoleCardsDealt(unsigned playerId, pkt::core::HoleCards holeCards)
{
    QString card1 = cardToString(holeCards.card1);
    QString card2 = cardToString(holeCards.card2);

    qDebug() << "QML: Hole cards dealt to player" << playerId << ":" << card1 << card2;

    // Cache hole cards for all players (needed for showdown reveal)
    m_cachedHoleCards[playerId] = holeCards;

    // Only show cards for human player (ID 0)
    if (playerId == 0)
    {
        m_viewModel->updatePlayerCards(playerId, card1, card2);
    }
    else
    {
        // Show card backs for other players
        m_viewModel->updatePlayerCards(playerId, "??", "??");
    }
}

void Bridge::handleBoardCardsDealt(pkt::core::BoardCards boardCards)
{
    qDebug() << "QML: Board cards dealt";

    QVariantList cards;
    // BoardCards has individual card fields, not an array
    if (boardCards.numCards >= 3)
    {
        cards.append(cardToString(boardCards.flop1));
        cards.append(cardToString(boardCards.flop2));
        cards.append(cardToString(boardCards.flop3));
    }
    if (boardCards.numCards >= 4)
    {
        cards.append(cardToString(boardCards.turn));
    }
    if (boardCards.numCards >= 5)
    {
        cards.append(cardToString(boardCards.river));
    }

    m_viewModel->setBoardCards(cards);
}

void Bridge::handleInvalidPlayerAction(unsigned playerId, pkt::core::PlayerAction invalidAction, std::string reason)
{
    qDebug() << "QML: Invalid action from player" << playerId << ":" << QString::fromStdString(reason);
    // Could show error message in QML UI
}

void Bridge::handleEngineError(std::string errorMessage)
{
    qDebug() << "QML: Engine error:" << QString::fromStdString(errorMessage);
    // Could show error dialog in QML UI
}

// Helper methods

QString Bridge::cardToString(const pkt::core::Card& card) const
{
    if (!card.isValid())
    {
        return "??";
    }

    // Use Card's built-in toString() method
    return QString::fromStdString(card.toString());
}

QString Bridge::gameStateToString(pkt::core::GameState state) const
{
    switch (state)
    {
    case GameState::None:
        return "Waiting";
    case GameState::Preflop:
        return "Pre-Flop";
    case GameState::Flop:
        return "Flop";
    case GameState::Turn:
        return "Turn";
    case GameState::River:
        return "River";
    case GameState::PostRiver:
        return "Post River";
    default:
        return "Unknown";
    }
}

QString Bridge::actionTypeToString(pkt::core::ActionType action) const
{
    switch (action)
    {
    case ActionType::Fold:
        return "Fold";
    case ActionType::Check:
        return "Check";
    case ActionType::Call:
        return "Call";
    case ActionType::Bet:
        return "Bet";
    case ActionType::Raise:
        return "Raise";
    case ActionType::Allin:
        return "All-In";
    case ActionType::PostSmallBlind:
        return "Small Blind";
    case ActionType::PostBigBlind:
        return "Big Blind";
    default:
        return "Unknown";
    }
}

void Bridge::setDealerFromSmallBlind(unsigned smallBlindId)
{
    if (m_playerCount <= 0)
        return;

    // Heads-up (2 players): Button is also Small Blind
    int dealerId = static_cast<int>(smallBlindId);

    if (m_playerCount > 2)
    {
        // 3+ players: Dealer is immediately to the right of Small Blind
        // (previous seat modulo N)
        dealerId = static_cast<int>(smallBlindId) - 1;
        if (dealerId < 0)
            dealerId = m_playerCount - 1;
    }

    qDebug() << "QML: Setting dealer position to player" << dealerId << "(small blind was player" << smallBlindId
             << ")";

    m_viewModel->setDealerPosition(dealerId);
}

} // namespace pkt::ui::qtqml::controller
