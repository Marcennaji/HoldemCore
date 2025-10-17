// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <QObject>
#include <core/engine/GameEvents.h>
#include <core/engine/cards/Card.h>
#include <core/engine/model/GameState.h>
#include <core/engine/model/PlayerAction.h>

namespace pkt::core
{
class Session;
namespace player
{
class HumanStrategy;
class Player;
} // namespace player
} // namespace pkt::core

namespace pkt::ui::qtqml::controller
{
class TableViewModel;

/**
 * @brief Bridge between the poker engine core and QML UI via TableViewModel.
 *
 * Handles communication between the game engine and the QML interface,
 * translating game events to view model updates and user interactions
 * back to the poker engine core.
 *
 * Similar to Bridge but updates a view model instead of direct UI.
 */
class Bridge : public QObject
{
    Q_OBJECT

  public:
    Bridge(pkt::core::Session* session, TableViewModel* viewModel, QObject* parent = nullptr);

    void connectEventsToViewModel(pkt::core::GameEvents& events);

  public slots:
    // Slots to handle user actions from QML UI
    void onPlayerFold();
    void onPlayerCall();
    void onPlayerCheck();
    void onPlayerBet(int amount);
    void onPlayerRaise(int amount);
    void onPlayerAllIn();
    void onNextHandRequested();

  private:
    // Event handlers for GameEvents - connected to game engine events
    void handlePlayersInitialized(const pkt::core::player::PlayerList& players);
    void handleGameInitialized(int gameSpeed);
    void handleHandCompleted(std::list<unsigned> winnerIds, int totalPot);
    void handleShowdownRevealOrder(const std::vector<unsigned>& revealOrder);
    void handlePlayerChipsUpdated(unsigned playerId, int newChips);
    void handleBettingRoundStarted(pkt::core::GameState gameState);
    void handlePotUpdated(int newPotAmount);
    void handlePlayerActed(pkt::core::PlayerAction action);
    void handleAwaitingHumanInput(unsigned playerId, std::vector<pkt::core::ActionType> validActions);
    void handleHoleCardsDealt(unsigned playerId, pkt::core::HoleCards holeCards);
    void handleBoardCardsDealt(pkt::core::BoardCards boardCards);
    void handleInvalidPlayerAction(unsigned playerId, pkt::core::PlayerAction invalidAction, std::string reason);
    void handleEngineError(std::string errorMessage);

    // Helper methods
    QString cardToString(const pkt::core::Card& card) const;
    QString gameStateToString(pkt::core::GameState state) const;
    QString actionTypeToString(pkt::core::ActionType action) const;

    TableViewModel* m_viewModel = nullptr;
    pkt::core::Session* m_session = nullptr;
    pkt::core::player::HumanStrategy* m_currentHumanStrategy = nullptr;
    int m_gameSpeed = 1;

    // Cache hole cards for all players for potential showdown reveal
    std::map<unsigned, pkt::core::HoleCards> m_cachedHoleCards;
};

} // namespace pkt::ui::qtqml::controller
