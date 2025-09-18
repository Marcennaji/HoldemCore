#include "Card.h"
#include <algorithm>
#include <stdexcept>

namespace pkt::core
{

Card::Card(int cardIndex) : m_cardIndex(cardIndex)
{
    if (cardIndex < -1 || cardIndex >= 52)
    {
        throw std::out_of_range("Card index out of range: " + std::to_string(cardIndex));
    }
}

Card::Card(Suit suit, Rank rank) : m_cardIndex(suitRankToIndex(suit, rank))
{
}

Card::Card(const std::string& cardString)
{
    if (!CardUtilities::isValidCardString(cardString))
    {
        throw std::invalid_argument("Invalid card string: " + cardString);
    }
    m_cardIndex = CardUtilities::getCardIndex(cardString);
}

Card::Suit Card::getSuit() const
{
    if (!isValid())
    {
        throw std::runtime_error("Cannot get suit of invalid card");
    }
    return indexToSuit(m_cardIndex);
}

Card::Rank Card::getRank() const
{
    if (!isValid())
    {
        throw std::runtime_error("Cannot get rank of invalid card");
    }
    return indexToRank(m_cardIndex);
}

std::string Card::toString() const
{
    if (!isValid())
    {
        return "Invalid";
    }
    return CardUtilities::getCardString(m_cardIndex);
}

int Card::getValue() const
{
    if (!isValid())
    {
        throw std::runtime_error("Cannot get value of invalid card");
    }
    return CardUtilities::getCardValue(toString());
}

char Card::getSuitChar() const
{
    if (!isValid())
    {
        throw std::runtime_error("Cannot get suit char of invalid card");
    }

    switch (getSuit())
    {
    case Suit::Diamonds:
        return 'd';
    case Suit::Hearts:
        return 'h';
    case Suit::Spades:
        return 's';
    case Suit::Clubs:
        return 'c';
    default:
        throw std::runtime_error("Unknown suit");
    }
}

char Card::getRankChar() const
{
    if (!isValid())
    {
        throw std::runtime_error("Cannot get rank char of invalid card");
    }

    const auto rank = getRank();
    switch (rank)
    {
    case Rank::Two:
        return '2';
    case Rank::Three:
        return '3';
    case Rank::Four:
        return '4';
    case Rank::Five:
        return '5';
    case Rank::Six:
        return '6';
    case Rank::Seven:
        return '7';
    case Rank::Eight:
        return '8';
    case Rank::Nine:
        return '9';
    case Rank::Ten:
        return 'T';
    case Rank::Jack:
        return 'J';
    case Rank::Queen:
        return 'Q';
    case Rank::King:
        return 'K';
    case Rank::Ace:
        return 'A';
    default:
        throw std::runtime_error("Unknown rank");
    }
}

Card::Suit Card::indexToSuit(int cardIndex)
{
    // Card index layout: 0-12=Diamonds, 13-25=Hearts, 26-38=Spades, 39-51=Clubs
    if (cardIndex >= 0 && cardIndex <= 12)
        return Suit::Diamonds;
    else if (cardIndex >= 13 && cardIndex <= 25)
        return Suit::Hearts;
    else if (cardIndex >= 26 && cardIndex <= 38)
        return Suit::Spades;
    else if (cardIndex >= 39 && cardIndex <= 51)
        return Suit::Clubs;
    else
        throw std::out_of_range("Invalid card index for suit conversion: " + std::to_string(cardIndex));
}

Card::Rank Card::indexToRank(int cardIndex)
{
    // Each suit has 13 cards: 2,3,4,5,6,7,8,9,T,J,Q,K,A
    const int rankIndex = cardIndex % 13;
    return static_cast<Rank>(rankIndex + 2); // Rank::Two = 2, so add 2 to 0-based index
}

int Card::suitRankToIndex(Suit suit, Rank rank)
{
    // Validate rank
    if (rank < Rank::Two || rank > Rank::Ace)
    {
        throw std::invalid_argument("Invalid rank: " + std::to_string(static_cast<int>(rank)));
    }

    // Calculate base index for suit
    int suitBase = 0;
    switch (suit)
    {
    case Suit::Diamonds:
        suitBase = 0;
        break;
    case Suit::Hearts:
        suitBase = 13;
        break;
    case Suit::Spades:
        suitBase = 26;
        break;
    case Suit::Clubs:
        suitBase = 39;
        break;
    default:
        throw std::invalid_argument("Invalid suit");
    }

    // Calculate rank offset (0-12)
    const int rankOffset = static_cast<int>(rank) - 2; // Rank::Two = 2, so subtract 2

    return suitBase + rankOffset;
}

} // namespace pkt::core