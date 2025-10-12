// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

// BoardArea.h
#pragma once

#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <array>

#include "core/engine/cards/Card.h"
#include "core/engine/model/GameState.h"

namespace pkt::ui::qtwidgets
{

/**
 * @brief Widget displaying the poker table's center area.
 * 
 * This widget encapsulates all UI components and logic for displaying:
 * - Pot amount
 * - Round state/phase (Preflop, Flop, Turn, River, Showdown)
 * - Community cards (0-5 cards)
 */
class BoardArea : public QGroupBox
{
    Q_OBJECT

  public:
    explicit BoardArea(QWidget* parent = nullptr);
    ~BoardArea() override = default;

    // Pot display
    void updatePot(int amount);

    // Round state display
    void updateRoundState(pkt::core::GameState gameState);
    void updateRoundState(const QString& text);

    // Community cards display
    void showCommunityCards(const pkt::core::BoardCards& boardCards);
    void clearCommunityCards();

    // Reset for new hand
    void reset();

  private:
    void setupUi();
    QString gameStateToText(pkt::core::GameState state) const;

    // UI Components
    QVBoxLayout* m_layout;
    QLabel* m_potLabel;
    QLabel* m_roundStateLabel;
    std::array<QLabel*, 5> m_communityCards;
};

} // namespace pkt::ui::qtwidgets
