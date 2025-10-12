// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


// PokerTableWindow.h
#pragma once

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QGroupBox>
#include <QWidget>
#include <QPixmap>
#include <QGraphicsOpacityEffect>
#include <array>
#include <memory>
#include <vector>
#include <list>

#include "core/engine/model/GameState.h"
#include "core/engine/model/GameData.h"
#include "core/engine/cards/Card.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/Player.h"
#include "core/session/Session.h"
#include "ui/qtwidgets/widgets/PlayerPanel.h"

namespace pkt::core
{
class Board;
} // namespace pkt::core

namespace pkt::ui::qtwidgets
{

/**
 * @brief Main window for the poker table user interface.
 * 
 * This class provides the graphical user interface for playing poker,
 * displaying the table, player positions, cards, betting controls,
 * and game state information. It handles user interactions and
 * communicates with the poker engine through the session.
 */
class PokerTableWindow : public QWidget
{
    Q_OBJECT

  public:
    explicit PokerTableWindow(pkt::core::Session* session, QWidget* parent = nullptr);
    
    // Initialize with GameData after it becomes available
    void initializeWithGameData(const pkt::core::GameData& gameData);
    ~PokerTableWindow() override = default;

    // Public interface for updating UI components
    void refresh();
    void refreshPot(int amount);
    void refreshPlayer(int seat, const pkt::core::player::Player& player);
    void showHoleCards(int seat, const pkt::core::HoleCards& holeCards);
    void showBoardCards(const pkt::core::BoardCards& boardCards);
    void updateGamePhase(pkt::core::GameState gameState);
    void updatePlayerStatus(int playerId, const QString& status);
  // Update only the stack amount for a player (used by onPlayerChipsUpdated)
  void updatePlayerCash(unsigned playerId, int newChips);
  // Robust, structured updates (preferred)
  void showPlayerAction(int playerId, pkt::core::ActionType action, int amount);
  void showPlayerTurn(int playerId);
  // Clear a single player's action label (used to clear human just before their turn)
  void clearPlayerActionLabel(int playerId);
    void showErrorMessage(const QString& message);
    
    // Game data access
    int getStartMoney() const { return m_startMoney; }
    void setAvailableActions(const std::vector<pkt::core::ActionType>& actions);
    void enablePlayerInput(bool enabled);
  // Reset bet controls (slider/spin) to default after an action
  void resetBetControls();
  // Show winners overlay label(s)
  void showWinners(const std::list<unsigned>& winnerIds, int totalPot);
  // Reveal hole cards at showdown in the exact order provided by the engine
  void revealShowdownOrder(const std::vector<unsigned>& revealOrder);
    
    // Player state indicators
    void setActivePlayer(int playerId);
    void setDealerPosition(int playerId);
  // Convenience: compute dealer from small blind id (dealer is left of SB)
  void setDealerFromSmallBlind(int smallBlindId);
    void clearPlayerHighlights();
    void updatePlayerStateIndicators();
  // Clear all players' action labels at new betting round
  void clearActionLabelsForNewRound();
    
    // Hand management
    void resetForNewHand();
    
    // Hand completion and delay functionality
    void onHandCompleted();

    // Cache and reveal helpers
    void cacheHoleCards(int seat, const pkt::core::HoleCards& holeCards);
    void setReachedShowdown(bool reached);
    
  // Track whether the full board to river was dealt (useful to infer showdown)
  inline bool sawRiver() const { return m_sawRiver; }

  signals:
    void playerActionRequested();
    void betClicked(int amount);
    void raiseClicked(int amount);
    void foldClicked();
    void callClicked();
    void checkClicked();
    void allInClicked();
    void nextHandRequested();  // Signal emitted when user clicks Next Hand button
  void windowClosed();       // Emitted when the table window is closed by the user

  private slots:
    void onBetAmountChanged(int amount);
    void onRaiseAction();
    void onNextHandClicked();  // Slot for Next Hand button click

  protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

  private:
    void setupUi();
    void connectSignals();
    void createPlayerAreas();
    void createActionButtons();
    void createBettingControls();
    // Compute the vertical space reserved at the bottom for controls
    int reservedBottomHeight() const;
    
    // Circular table layout helpers
    void positionPlayersInCircle();
    QPoint calculateCircularPosition(int playerIndex, int totalPlayers, const QPoint& center, int radius);
    void createCenterArea();
    void positionCenterArea();
  void positionDealerButtons();
    
    // Card visualization helpers
    QPixmap getCardPixmap(const pkt::core::Card& card) const;
    QPixmap getCardBackPixmap() const;
    QString getCardImagePath(int cardIndex) const;

    // UI Organization
    pkt::core::Session* m_session;
    
    // Main layout
  QVBoxLayout* m_mainLayout = nullptr;
  QGridLayout* m_tableLayout = nullptr;
    
    // Player panels (all players including human in circular layout)
    std::vector<PlayerPanel*> m_playerPanels;
    int m_maxPlayers;
    int m_startMoney; // Starting cash amount for current game
    
    // Player state tracking
    int m_activePlayerId;   // Currently active player (-1 if none)
    int m_dealerPosition;   // Current dealer position (-1 if none)
    
    // Center area for pot, phase, and community cards
  QGroupBox* m_centerArea = nullptr;
  QLabel* m_potLabel = nullptr;
  QLabel* m_roundStateLabel = nullptr;
  std::array<QLabel*, 5> m_communityCards{};

    // Action controls
  QGroupBox* m_actionGroup = nullptr;
  QHBoxLayout* m_actionLayout = nullptr;
  QPushButton* m_foldButton = nullptr;
  QPushButton* m_callButton = nullptr;
  QPushButton* m_checkButton = nullptr;
  QPushButton* m_betButton = nullptr;
  QPushButton* m_raiseButton = nullptr;
  QPushButton* m_allInButton = nullptr;
    
    // Betting controls
  QGroupBox* m_bettingGroup = nullptr;
  QHBoxLayout* m_bettingLayout = nullptr;
  QSlider* m_betSlider = nullptr;
  QSpinBox* m_betSpinBox = nullptr;
  QLabel* m_betAmountLabel = nullptr;
    
    // Next hand control
  QPushButton* m_nextHandButton = nullptr;

    // Cached state for showdown reveal
    std::vector<pkt::core::HoleCards> m_cachedHoleCards;
    bool m_reachedShowdown = false;
  bool m_sawRiver = false;
};
} // namespace pkt::ui::qtwidgets
