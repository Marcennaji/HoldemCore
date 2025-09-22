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
#include <array>
#include <memory>

#include "core/engine/model/GameState.h"
#include "core/engine/cards/Card.h"
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
    ~PokerTableWindow() override = default;

    // Public interface for updating UI components
    void refresh();
    void refreshPot(int amount);
    void refreshPlayer(int seat, const pkt::core::player::Player& player);
    void showHoleCards(int seat, const pkt::core::HoleCards& holeCards);
    void showBoardCards(const pkt::core::BoardCards& boardCards);
    void updateGamePhase(pkt::core::GameState gameState);
    void updatePlayerStatus(int playerId, const QString& status);
    void showErrorMessage(const QString& message);
    void setAvailableActions(const std::vector<pkt::core::ActionType>& actions);
    void enablePlayerInput(bool enabled);

  signals:
    void playerActionRequested();
    void betClicked(int amount);
    void raiseClicked(int amount);
    void foldClicked();
    void callClicked();
    void checkClicked();
    void allInClicked();

  private slots:
    void onBetAmountChanged(int amount);
    void onRaiseAction();

  private:
    void setupUi();
    void connectSignals();
    void createGameInfoArea();
    void createPlayerAreas();
    void createBoardArea();
    void createActionButtons();
    void createBettingControls();
    
    // Card visualization helpers
    QPixmap getCardPixmap(const pkt::core::Card& card) const;
    QPixmap getCardBackPixmap() const;
    QString getCardImagePath(int cardIndex) const;

    // UI Organization
    pkt::core::Session* m_session;
    
    // Main layout
    QVBoxLayout* m_mainLayout;
    QGridLayout* m_tableLayout;
    
    // Game info area
    QGroupBox* m_gameInfoGroup;
    QHBoxLayout* m_gameInfoLayout;
    QLabel* m_potLabel;
    QLabel* m_gamePhaseLabel;
    QLabel* m_statusLabel;
    
    // Player areas (up to 9 players)
    struct PlayerUIComponents {
        QGroupBox* playerGroup;
        QLabel* nameLabel;
        QLabel* chipsLabel;
        QLabel* statusLabel;
        QLabel* holeCard1;
        QLabel* holeCard2;
    };
    std::array<PlayerUIComponents, 9> m_playerComponents;
    
    // Board area
    QGroupBox* m_boardGroup;
    QHBoxLayout* m_boardLayout;
    std::array<QLabel*, 5> m_boardCardLabels;

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
};
} // namespace pkt::ui::qtwidgets
