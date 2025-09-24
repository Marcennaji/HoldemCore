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

#include "core/engine/model/GameState.h"
#include "core/engine/model/GameData.h"
#include "core/engine/cards/Card.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/Player.h"
#include "core/session/Session.h"

namespace pkt::core
{
class IBoard;
} // namespace pkt::core

namespace pkt::ui::qtwidgets
{
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
  // Robust, structured updates (preferred)
  void showPlayerAction(int playerId, pkt::core::ActionType action, int amount);
  void showPlayerTurn(int playerId);
    void showErrorMessage(const QString& message);
    void setAvailableActions(const std::vector<pkt::core::ActionType>& actions);
    void enablePlayerInput(bool enabled);
  // Reset bet controls (slider/spin) to default after an action
  void resetBetControls();
    
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

  signals:
    void playerActionRequested();
    void betClicked(int amount);
    void raiseClicked(int amount);
    void foldClicked();
    void callClicked();
    void checkClicked();
    void allInClicked();
    void nextHandRequested();  // Signal emitted when user clicks Next Hand button

  private slots:
    void onBetAmountChanged(int amount);
    void onRaiseAction();
    void onNextHandClicked();  // Slot for Next Hand button click

  protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

  private:
    void setupUi();
    void connectSignals();
    void createPlayerAreas();
    void createActionButtons();
    void createBettingControls();
    // Styling helpers (avoid duplication)
    QString defaultPlayerGroupStyle() const;
    QString activePlayerGroupStyle() const;
    QString currentActionLabelStyleBase() const;
    QString currentActionLabelStyleFor(const QString& action) const;
    
    // Circular table layout helpers
    void positionPlayersInCircle();
    QPoint calculateCircularPosition(int playerIndex, int totalPlayers, const QPoint& center, int radius);
    void createCenterArea();
    void positionCenterArea();
    
    // Card visualization helpers
    QPixmap getCardPixmap(const pkt::core::Card& card) const;
    QPixmap getCardBackPixmap() const;
    QString getCardImagePath(int cardIndex) const;

    // UI Organization
    pkt::core::Session* m_session;
    
    // Main layout
    QVBoxLayout* m_mainLayout;
    QGridLayout* m_tableLayout;
    
    // Player areas (all players including human in circular layout)
    struct PlayerUIComponents {
        QGroupBox* playerGroup;
        QLabel* holeCard1;
        QLabel* holeCard2;
        QLabel* dealerButton;  // Dealer button indicator
    QLabel* currentActionLabel; // Prominent action text
    QGraphicsOpacityEffect* card1OpacityEffect;
    QGraphicsOpacityEffect* card2OpacityEffect;
    bool isFolded = false;
        bool isHuman;  // True for human player (index 0)
    };
    std::vector<PlayerUIComponents> m_playerComponents;
    int m_maxPlayers;
    
    // Player state tracking
    int m_activePlayerId;   // Currently active player (-1 if none)
    int m_dealerPosition;   // Current dealer position (-1 if none)
    
    // Center area for pot, phase, and community cards
    QGroupBox* m_centerArea;
    QLabel* m_potLabel;
    QLabel* m_roundStateLabel;
    std::array<QLabel*, 5> m_communityCards;

    // Action controls
    QGroupBox* m_actionGroup;
    QHBoxLayout* m_actionLayout;
    QPushButton* m_foldButton;
    QPushButton* m_callButton;
    QPushButton* m_checkButton;
    QPushButton* m_betButton;
    QPushButton* m_raiseButton;
    QPushButton* m_allInButton;
    
    // Betting controls
    QGroupBox* m_bettingGroup;
    QHBoxLayout* m_bettingLayout;
    QSlider* m_betSlider;
    QSpinBox* m_betSpinBox;
    QLabel* m_betAmountLabel;
    
    // Next hand control
    QPushButton* m_nextHandButton;
    
  // Helpers
  void applyFoldVisual(int seat, bool folded);
};
} // namespace pkt::ui::qtwidgets
