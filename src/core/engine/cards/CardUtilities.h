// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once
#include <map>
#include <string>
#include <vector>

namespace pkt::core
{

/**
 * @brief Utility functions for card string conversions and representations.
 * 
 * Provides static methods for converting between card indices and
 * human-readable string representations, supporting both individual
 * cards and collections of cards.
 */
/// Utility class for card string representations and conversions
class CardUtilities
{
  public:
    /// Converts card indices to human-readable string representation.
    /// \param cardValues Vector of card indices.
    /// \return Space-separated string of card representations.
    static std::string getCardStringValue(const std::vector<int>& cardValues);

    /// Converts card indices to human-readable string representation.
    /// \param cardValues Array of card indices.
    /// \param size Number of cards in the array.
    /// \return Space-separated string of card representations.
    static std::string getCardStringValue(const int* cardValues, int size);

    /// Gets card string representation for a single card index
    /// \param cardIndex Card index (0-51)
    /// \return Card string (e.g., "Ah", "Kd")
    static const std::string& getCardString(int cardIndex);

    /// Gets card index from string representation
    /// \param cardString Card string (e.g., "Ah", "Kd")
    /// \return Card index (0-51)
    static int getCardIndex(const std::string& cardString);

    /// Gets the numeric value of a card for comparison (2-14)
    /// \param cardString Card string (e.g., "Ah", "Kd")
    /// \return Card value (2-14) where 2=deuce, 14=ace
    static int getCardValue(const std::string& cardString);

    /// Validates if a card string is valid
    /// \param cardString Card string to validate
    /// \return True if valid, false otherwise
    static bool isValidCardString(const std::string& cardString);

  private:
    /// Array of card string representations indexed by card index
    static const std::string CARD_STRING_VALUES[];

    /// Map from card string to card index for fast lookup
    static const std::map<std::string, int> CARD_STRING_TO_INDEX;

    /// Map from card string to card value for ordering (2-14)
    static const std::map<std::string, int> CARD_STRING_ORDERING;
};

} // namespace pkt::core