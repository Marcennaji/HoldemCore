// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <QObject>
#include <QString>

namespace pkt::ui::qtqml::controller
{

/**
 * @brief Utility class exposing card conversion functions to QML.
 *
 * Provides Q_INVOKABLE static methods that allow QML to use the same
 * card indexing logic as the engine, ensuring consistency between
 * card string representations and their numeric indices.
 *
 * This eliminates the need to duplicate card indexing logic in QML JavaScript,
 * maintaining a single source of truth in the C++ engine code.
 */
class CardHelper : public QObject
{
    Q_OBJECT

  public:
    explicit CardHelper(QObject* parent = nullptr) : QObject(parent) {}

    /**
     * @brief Convert card string to card index using engine's indexing scheme.
     *
     * Uses the engine's CardUtilities::getCardIndex() to ensure consistency.
     * Card indexing layout (as defined in Card.cpp):
     * - 0-12:   2d-Ad (Diamonds)
     * - 13-25:  2h-Ah (Hearts)
     * - 26-38:  2s-As (Spades)
     * - 39-51:  2c-Ac (Clubs)
     *
     * @param cardString Card notation (e.g., "As", "Kh", "2d")
     * @return Card index (0-51), or -1 if invalid
     */
    Q_INVOKABLE static int getCardIndex(const QString& cardString);

    /**
     * @brief Convert card index to card string using engine's indexing scheme.
     *
     * Uses the engine's CardUtilities::getCardString() to ensure consistency.
     *
     * @param cardIndex Card index (0-51)
     * @return Card notation (e.g., "As", "Kh", "2d"), or "Invalid" if out of range
     */
    Q_INVOKABLE static QString getCardString(int cardIndex);

    /**
     * @brief Validate if a card string is valid.
     *
     * @param cardString Card notation to validate
     * @return True if valid card string, false otherwise
     */
    Q_INVOKABLE static bool isValidCard(const QString& cardString);
};

} // namespace pkt::ui::qtqml::controller
