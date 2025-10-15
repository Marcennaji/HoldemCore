// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

// PlayerPanel.h
#pragma once

#include <QGraphicsOpacityEffect>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

#include "core/engine/cards/Card.h"
#include "core/engine/model/PlayerAction.h"

namespace pkt::ui::qtwidgets
{

/**
 * @brief Widget displaying a single player's information in the poker table.
 *
 * This widget encapsulates all UI components and logic for displaying:
 * - Player name and chip count
 * - Hole cards (face up or face down)
 * - Current action label (e.g., "FOLD", "CALL $50")
 * - Dealer button indicator
 * - Winner badge
 * - Visual states (active, folded)
 */
class PlayerPanel : public QGroupBox
{
    Q_OBJECT

  public:
    /**
     * @brief Construct a new Player Panel
     * @param playerId The player's unique identifier
     * @param isHuman True if this is the human player
     * @param parent Parent widget
     */
    explicit PlayerPanel(int playerId, bool isHuman, QWidget* parent = nullptr);
    ~PlayerPanel() override = default;

    // Player information updates
    void updatePlayerInfo(const QString& name, const QString& strategyName, int chips);
    void updateChips(int chips);

    // Card display
    void showHoleCards(const pkt::core::HoleCards& cards);
    void hideHoleCards();
    void showCardBack();

    // Action display
    void showAction(pkt::core::ActionType action, int amount);
    void clearAction();

    // Visual states
    void setActive(bool active);
    void setDealer(bool isDealer);
    void setFolded(bool folded);
    void showWinner(bool isWinner);

    // Reset for new hand
    void reset();

    // Dealer button positioning (called by parent after layout)
    QLabel* dealerButton() { return m_dealerButton; }

    // Query state
    int playerId() const { return m_playerId; }
    bool isHuman() const { return m_isHuman; }
    bool isFolded() const { return m_isFolded; }

  private:
    void setupUi();
    void applyFoldVisual();
    void clearFoldVisual();

    QString currentActionLabelStyleBase() const;
    QString currentActionLabelStyleFor(const QString& action) const;
    QString defaultPlayerGroupStyle() const;
    QString activePlayerGroupStyle() const;

    // Player identity
    int m_playerId;
    bool m_isHuman;
    QString m_playerName;
    int m_currentChips;

    // State
    bool m_isFolded;
    bool m_isActive;

    // UI Components
    QVBoxLayout* m_layout;
    QLabel* m_holeCard1;
    QLabel* m_holeCard2;
    QLabel* m_currentActionLabel;
    QLabel* m_winnerLabel;
    QLabel* m_dealerButton; // Positioned by parent, not in layout

    // Visual effects
    QGraphicsOpacityEffect* m_card1OpacityEffect;
    QGraphicsOpacityEffect* m_card2OpacityEffect;
};

} // namespace pkt::ui::qtwidgets
