#pragma once

#include <string>
#include "core/engine/CardUtilities.h"

namespace pkt::core
{

/// Represents a playing card with suit and rank.
/// Wraps the existing int-based card representation to maintain compatibility.
class Card
{
  public:
    /// Card suits
    enum class Suit
    {
        Diamonds = 0, // 'd'
        Hearts = 1,   // 'h'
        Spades = 2,   // 's'
        Clubs = 3     // 'c'
    };

    /// Card ranks (values)
    enum class Rank
    {
        Two = 2,
        Three = 3,
        Four = 4,
        Five = 5,
        Six = 6,
        Seven = 7,
        Eight = 8,
        Nine = 9,
        Ten = 10,
        Jack = 11,
        Queen = 12,
        King = 13,
        Ace = 14
    };

    /// Default constructor - creates invalid card
    Card() : m_cardIndex(-1) {}

    /// Constructor from card index (0-51)
    /// \param cardIndex Card index compatible with existing system
    explicit Card(int cardIndex);

    /// Constructor from suit and rank
    /// \param suit Card suit
    /// \param rank Card rank
    Card(Suit suit, Rank rank);

    /// Constructor from string representation (e.g., "Ah", "Kd")
    /// \param cardString String representation
    explicit Card(const std::string& cardString);

    /// Copy constructor
    Card(const Card& other) = default;

    /// Assignment operator
    Card& operator=(const Card& other) = default;

    /// Get the card index (0-51) for compatibility with existing code
    /// \return Card index compatible with existing int-based system
    int getIndex() const { return m_cardIndex; }

    /// Get card suit
    /// \return Card suit
    Suit getSuit() const;

    /// Get card rank
    /// \return Card rank
    Rank getRank() const;

    /// Get string representation (e.g., "Ah", "Kd")
    /// \return String representation
    std::string toString() const;

    /// Get card value for comparison (2-14)
    /// \return Card value where 2=deuce, 14=ace
    int getValue() const;

    /// Check if card is valid
    /// \return True if card is valid, false otherwise
    bool isValid() const { return m_cardIndex >= 0 && m_cardIndex < 52; }

    /// Implicit conversion to int for backward compatibility
    operator int() const { return m_cardIndex; }

    /// Equality comparison
    bool operator==(const Card& other) const { return m_cardIndex == other.m_cardIndex; }

    /// Inequality comparison
    bool operator!=(const Card& other) const { return m_cardIndex != other.m_cardIndex; }

    /// Less than comparison (by card index)
    bool operator<(const Card& other) const { return m_cardIndex < other.m_cardIndex; }

    /// Static method to create Card from existing int representation
    /// \param cardIndex Existing card index
    /// \return Card object
    static Card fromIndex(int cardIndex) { return Card(cardIndex); }

    /// Get suit character ('d', 'h', 's', 'c')
    /// \return Suit character
    char getSuitChar() const;

    /// Get rank character ('2'-'9', 'T', 'J', 'Q', 'K', 'A')
    /// \return Rank character
    char getRankChar() const;

  private:
    int m_cardIndex; ///< Internal card index (0-51) for compatibility

    /// Convert card index to suit
    /// \param cardIndex Card index
    /// \return Suit
    static Suit indexToSuit(int cardIndex);

    /// Convert card index to rank
    /// \param cardIndex Card index
    /// \return Rank
    static Rank indexToRank(int cardIndex);

    /// Convert suit and rank to card index
    /// \param suit Card suit
    /// \param rank Card rank
    /// \return Card index
    static int suitRankToIndex(Suit suit, Rank rank);
};

} // namespace pkt::core