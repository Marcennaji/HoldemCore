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

namespace pkt::ui::qtwidgets
{
class PokerTableWindow;
struct PlayerDisplayInfo;
} // namespace pkt::ui::qtwidgets

namespace pkt::ui::qtwidgets::controller
{

/**
 * @brief Bridge between the poker engine core and Qt Widgets GUI components.
 *
 * Handles communication between the game engine and the user interface,
 * translating game events to GUI updates and user interactions back to
 * the poker engine core.
 */
class Bridge : public QObject
{
    Q_OBJECT

  public:
    Bridge(pkt::core::Session* session, pkt::ui::qtwidgets::PokerTableWindow* pokerTableWindow,
           QObject* parent = nullptr);

    void connectEventsToUi(pkt::core::GameEvents& events);

  private slots:
    void connectSignalsFromUi();

    // Slots to handle user actions from UI
    void onPlayerFold();
    void onPlayerCall();
    void onPlayerCheck();
    void onPlayerBet(int amount);
    void onPlayerRaise(int amount);
    void onPlayerAllIn();
    void onNextHandRequested();

  private:
    // Event handlers for GameEvents - these will be connected to the game engine events
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

    pkt::ui::qtwidgets::PokerTableWindow* m_tableWindow = nullptr;
    pkt::core::Session* m_session = nullptr;

    // Track the human strategy that's currently waiting for input
    pkt::core::player::HumanStrategy* m_currentHumanStrategy = nullptr;

    // Game speed for UI pacing (higher = faster; used to compute delay between bot actions)
    int m_gameSpeed = 1;

    // Track human player's last known chips to enforce auto-fold at 0
    int m_humanChips = -1;

    // Helper: auto-fold human if out of chips
    void autoFoldHumanIfBroke();

    // Compute UI delay in milliseconds based on m_gameSpeed (range: 500ms..3000ms)
    int computeDelayMsForBots() const;

    // Helper: Convert Player facade to lightweight DTO for UI display
    pkt::ui::qtwidgets::PlayerDisplayInfo createPlayerDisplayInfo(const pkt::core::player::Player& player) const;
};

} // namespace pkt::ui::qtwidgets::controller
