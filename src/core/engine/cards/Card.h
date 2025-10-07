#pragma once

#include "core/engine/cards/CardUtilities.h"
#include "core/interfaces/Randomizer.h"

#include <algorithm>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

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

/// Structure to hold a player's hole cards - Card-first design for new FSM code
struct HoleCards
{
    Card card1;
    Card card2;

    /// Default constructor - creates invalid cards
    HoleCards() = default;

    /// Constructor from two Card objects (preferred for new code)
    HoleCards(const Card& c1, const Card& c2) : card1(c1), card2(c2) {}

    /// Constructor from card strings (convenient for new code)
    HoleCards(const std::string& card1Str, const std::string& card2Str) : card1(card1Str), card2(card2Str) {}

    /// Constructor from int array (only for legacy interface compatibility)
    HoleCards(const int cardIndices[2]) : card1(cardIndices[0]), card2(cardIndices[1]) {}

    /// Check if both cards are valid
    bool isValid() const { return card1.isValid() && card2.isValid(); }

    /// Get combined string representation "Ah Ks"
    std::string toString() const
    {
        if (!isValid())
            return "Invalid Cards";
        return card1.toString() + " " + card2.toString();
    }

    /// Convert to int array for legacy interface compatibility
    void toIntArray(int cardIndices[2]) const
    {
        cardIndices[0] = card1.getIndex();
        cardIndices[1] = card2.getIndex();
    }

    /// Set from int array for legacy interface compatibility
    void fromIntArray(const int cardIndices[2])
    {
        card1 = Card::fromIndex(cardIndices[0]);
        card2 = Card::fromIndex(cardIndices[1]);
    }

    /// Reset to invalid cards
    void reset()
    {
        card1 = Card();
        card2 = Card();
    }
};

/// BoardCards represents the 5 community cards on the board (flop, turn, river)
/// Valid states: 0 cards (preflop), 3 cards (flop), 4 cards (turn), 5 cards (river)
struct BoardCards
{
    Card flop1, flop2, flop3, turn, river;
    int numCards; // Track how many cards are actually set (0, 3, 4, or 5)

    /// Default constructor - creates preflop state (0 cards)
    BoardCards() : flop1(), flop2(), flop3(), turn(), river(), numCards(0) {}

    /// Constructor from three Card objects for flop
    BoardCards(const Card& f1, const Card& f2, const Card& f3)
        : flop1(f1), flop2(f2), flop3(f3), turn(), river(), numCards(3)
    {
    }

    /// Constructor from four Card objects for turn
    BoardCards(const Card& f1, const Card& f2, const Card& f3, const Card& t)
        : flop1(f1), flop2(f2), flop3(f3), turn(t), river(), numCards(4)
    {
    }

    /// Constructor from five Card objects for river
    BoardCards(const Card& f1, const Card& f2, const Card& f3, const Card& t, const Card& r)
        : flop1(f1), flop2(f2), flop3(f3), turn(t), river(r), numCards(5)
    {
    }

    /// Constructor from card strings for flop
    BoardCards(const std::string& f1Str, const std::string& f2Str, const std::string& f3Str)
        : flop1(f1Str), flop2(f2Str), flop3(f3Str), turn(), river(), numCards(3)
    {
    }

    /// Constructor from card strings for turn
    BoardCards(const std::string& f1Str, const std::string& f2Str, const std::string& f3Str, const std::string& tStr)
        : flop1(f1Str), flop2(f2Str), flop3(f3Str), turn(tStr), river(), numCards(4)
    {
    }

    /// Constructor from card strings for river
    BoardCards(const std::string& f1Str, const std::string& f2Str, const std::string& f3Str, const std::string& tStr,
               const std::string& rStr)
        : flop1(f1Str), flop2(f2Str), flop3(f3Str), turn(tStr), river(rStr), numCards(5)
    {
    }

    /// Constructor from int array (only for legacy interface compatibility)
    /// Automatically determines number of cards based on valid card indices
    BoardCards(const int cardIndices[5]) : flop1(), flop2(), flop3(), turn(), river(), numCards(0)
    {
        fromIntArray(cardIndices);
    }

    /// Check if board state is valid (0, 3, 4, or 5 cards)
    bool isValid() const
    {
        if (numCards == 0)
            return true; // Preflop is valid
        if (numCards == 3)
            return flop1.isValid() && flop2.isValid() && flop3.isValid();
        if (numCards == 4)
            return flop1.isValid() && flop2.isValid() && flop3.isValid() && turn.isValid();
        if (numCards == 5)
            return flop1.isValid() && flop2.isValid() && flop3.isValid() && turn.isValid() && river.isValid();
        return false; // Invalid number of cards
    }

    /// Get number of cards currently on board
    int getNumCards() const { return numCards; }

    /// Check if it's preflop (no cards)
    bool isPreflop() const { return numCards == 0; }

    /// Check if flop is dealt
    bool isFlop() const { return numCards == 3; }

    /// Check if turn is dealt
    bool isTurn() const { return numCards == 4; }

    /// Check if river is dealt
    bool isRiver() const { return numCards == 5; }

    /// Get combined string representation based on current game phase
    std::string toString() const
    {
        if (numCards == 0)
            return "<no cards>";
        if (numCards == 3)
            return flop1.toString() + " " + flop2.toString() + " " + flop3.toString();
        if (numCards == 4)
            return flop1.toString() + " " + flop2.toString() + " " + flop3.toString() + " " + turn.toString();
        if (numCards == 5)
            return flop1.toString() + " " + flop2.toString() + " " + flop3.toString() + " " + turn.toString() + " " +
                   river.toString();
        return "Invalid Board State";
    }

    /// Convert to int array for legacy interface compatibility
    /// Always provides all 5 cards to maintain backward compatibility with engine
    void toIntArray(int cardIndices[5]) const
    {
        cardIndices[0] = flop1.getIndex();
        cardIndices[1] = flop2.getIndex();
        cardIndices[2] = flop3.getIndex();
        cardIndices[3] = turn.getIndex();
        cardIndices[4] = river.getIndex();
    }

    /// Set from int array for legacy interface compatibility
    /// Always expects all 5 cards to maintain backward compatibility
    void fromIntArray(const int cardIndices[5])
    {
        flop1 = Card::fromIndex(cardIndices[0]);
        flop2 = Card::fromIndex(cardIndices[1]);
        flop3 = Card::fromIndex(cardIndices[2]);
        turn = Card::fromIndex(cardIndices[3]);
        river = Card::fromIndex(cardIndices[4]);
        numCards = 5; // Legacy interface always provides all 5 cards
    }

    /// Reset to preflop state (0 cards)
    void reset()
    {
        flop1 = Card();
        flop2 = Card();
        flop3 = Card();
        turn = Card();
        river = Card();
        numCards = 0;
    }

    /// Deal flop (3 cards)
    void dealFlop(const Card& f1, const Card& f2, const Card& f3)
    {
        if (numCards != 0)
            return; // Can only deal flop from preflop
        flop1 = f1;
        flop2 = f2;
        flop3 = f3;
        numCards = 3;
    }

    /// Deal turn (add 4th card)
    void dealTurn(const Card& t)
    {
        if (numCards != 3)
            return; // Can only deal turn after flop
        turn = t;
        numCards = 4;
    }

    /// Deal river (add 5th card)
    void dealRiver(const Card& r)
    {
        if (numCards != 4)
            return; // Can only deal river after turn
        river = r;
        numCards = 5;
    }

    /// Get card by index (0-4) for iteration - only returns valid cards
    const Card& getCard(int index) const
    {
        if (index < 0 || index >= numCards)
        {
            static Card invalid;
            return invalid;
        }

        switch (index)
        {
        case 0:
            return flop1;
        case 1:
            return flop2;
        case 2:
            return flop3;
        case 3:
            return turn;
        case 4:
            return river;
        default:
            static Card invalid;
            return invalid;
        }
    }
};

/// Deck represents a standard 52-card poker deck with shuffling and dealing capabilities
class Deck
{
  private:
    std::vector<Card> cards;
    size_t nextCardIndex; // Index of next card to deal

  public:
    /// Default constructor - creates a full 52-card deck in order
    Deck() : nextCardIndex(0) { initializeFullDeck(); }

    /// Initialize with a full 52-card deck in standard order
    void initializeFullDeck()
    {
        cards.clear();
        cards.reserve(52);

        // Create all 52 cards in order: 2h, 3h, ..., Ah, 2d, 3d, ..., As
        for (int suit = 0; suit < 4; ++suit)
        {
            for (int rank = 0; rank < 13; ++rank)
            {
                // Convert 0-based rank to actual Rank enum (Two=2, Three=3, ..., Ace=14)
                Card::Rank cardRank = static_cast<Card::Rank>(rank + 2);
                cards.emplace_back(static_cast<Card::Suit>(suit), cardRank);
            }
        }
        nextCardIndex = 0;
    }

    /// Shuffle the deck using Randomizer interface (ISP-compliant)
    void shuffle(std::shared_ptr<Randomizer> randomizer)
    {
        if (!randomizer) {
            throw std::runtime_error("Randomizer service is required for deck shuffle");
        }

        // Use a simple shuffle algorithm that uses our randomizer
        for (size_t i = cards.size() - 1; i > 0; --i)
        {
            int randomValues[1];
            randomizer->getRand(0, static_cast<int>(i), 1, randomValues);
            size_t j = static_cast<size_t>(randomValues[0]);
            std::swap(cards[i], cards[j]);
        }
        nextCardIndex = 0; // Reset deal position after shuffle
    }

    /// Deal the next card from the deck
    /// Throws std::runtime_error if no cards left
    Card dealCard()
    {
        if (nextCardIndex >= cards.size())
        {
            throw std::runtime_error("No more cards left in deck to deal");
        }
        return cards[nextCardIndex++];
    }

    /// Deal multiple cards at once
    /// Throws std::runtime_error if not enough cards left
    std::vector<Card> dealCards(size_t count)
    {
        if (nextCardIndex + count > cards.size())
        {
            throw std::runtime_error("Not enough cards left in deck to deal " + std::to_string(count) + " cards");
        }

        std::vector<Card> dealtCards;
        dealtCards.reserve(count);
        for (size_t i = 0; i < count; ++i)
        {
            dealtCards.push_back(cards[nextCardIndex++]);
        }
        return dealtCards;
    }

    /// Get number of cards remaining in deck
    size_t remainingCards() const { return cards.size() - nextCardIndex; }

    /// Check if enough cards remain to deal to all players and board
    /// boardCards: number of board cards needed (0, 3, 4, or 5)
    /// numPlayers: number of players needing hole cards
    bool hasEnoughCards(size_t boardCards, size_t numPlayers) const
    {
        return remainingCards() >= boardCards + (numPlayers * 2);
    }

    /// Reset the deck to start dealing from beginning (without reshuffling)
    void resetDealPosition() { nextCardIndex = 0; }

    /// Get all cards in deck (for testing/debugging)
    const std::vector<Card>& getAllCards() const { return cards; }

    /// Get legacy int representation of card at specific index (for compatibility)
    int getLegacyCardIndex(size_t deckIndex) const
    {
        if (deckIndex >= cards.size())
        {
            throw std::runtime_error("Invalid deck index: " + std::to_string(deckIndex));
        }
        return cards[deckIndex].getIndex();
    }
};

} // namespace pkt::core