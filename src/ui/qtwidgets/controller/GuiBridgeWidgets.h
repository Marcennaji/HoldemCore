#pragma once

#include <QObject>
#include <core/engine/GameEvents.h>
#include <core/engine/cards/Card.h>
#include <core/engine/model/PlayerAction.h>
#include <core/engine/model/GameState.h>

class PokerTableWindow;

namespace pkt::core
{
class Session;
namespace player
{
class HumanStrategy;
}
}

namespace pkt::ui::qtwidgets
{
class PokerTableWindow;

class GuiBridgeWidgets : public QObject
{
    Q_OBJECT

  public:
    GuiBridgeWidgets(pkt::core::Session* session, PokerTableWindow* pokerTableWindow, QObject* parent = nullptr);
    
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

    PokerTableWindow* myTableWindow = nullptr;
    pkt::core::Session* mySession = nullptr;
    
    // Track the human strategy that's currently waiting for input
    pkt::core::player::HumanStrategy* myCurrentHumanStrategy = nullptr;

  // Game speed for UI pacing (higher = faster; used to compute delay between bot actions)
  int m_gameSpeed = 1;

  // Track human player's last known chips to enforce auto-fold at 0
  int m_humanChips = -1;

  // Helper: auto-fold human if out of chips
  void autoFoldHumanIfBroke();

  // Compute UI delay in milliseconds based on m_gameSpeed (range: 500ms..3000ms)
  int computeDelayMsForBots() const;
};

} // namespace pkt::ui::qtwidgets
