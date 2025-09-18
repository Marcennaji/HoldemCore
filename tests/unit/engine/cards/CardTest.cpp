#include "core/engine/cards/Card.h"
#include <gtest/gtest.h>
#include "core/engine/cards/CardUtilities.h"

using namespace pkt::core;

class CardTest : public ::testing::Test
{
  protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(CardTest, DefaultConstructorCreatesInvalidCard)
{
    Card card;
    EXPECT_FALSE(card.isValid());
    EXPECT_EQ(card.getIndex(), -1);
}

TEST_F(CardTest, ConstructorFromValidIndex)
{
    Card card(0); // 2d - first card
    EXPECT_TRUE(card.isValid());
    EXPECT_EQ(card.getIndex(), 0);
    EXPECT_EQ(card.getSuit(), Card::Suit::Diamonds);
    EXPECT_EQ(card.getRank(), Card::Rank::Two);
}

TEST_F(CardTest, ConstructorFromInvalidIndex)
{
    EXPECT_THROW(Card card(-2), std::out_of_range);
    EXPECT_THROW(Card card(52), std::out_of_range);
}

TEST_F(CardTest, ConstructorFromSuitAndRank)
{
    Card card(Card::Suit::Hearts, Card::Rank::Ace);
    EXPECT_TRUE(card.isValid());
    EXPECT_EQ(card.getSuit(), Card::Suit::Hearts);
    EXPECT_EQ(card.getRank(), Card::Rank::Ace);
    EXPECT_EQ(card.toString(), "Ah");
}

TEST_F(CardTest, ConstructorFromString)
{
    Card card("Kd");
    EXPECT_TRUE(card.isValid());
    EXPECT_EQ(card.getSuit(), Card::Suit::Diamonds);
    EXPECT_EQ(card.getRank(), Card::Rank::King);
    EXPECT_EQ(card.toString(), "Kd");
}

TEST_F(CardTest, ConstructorFromInvalidString)
{
    EXPECT_THROW(Card card("XX"), std::invalid_argument);
    EXPECT_THROW(Card card(""), std::invalid_argument);
}

TEST_F(CardTest, ToString)
{
    Card card(Card::Suit::Spades, Card::Rank::Queen);
    EXPECT_EQ(card.toString(), "Qs");

    Card invalidCard;
    EXPECT_EQ(invalidCard.toString(), "Invalid");
}

TEST_F(CardTest, GetValue)
{
    Card twoOfHearts(Card::Suit::Hearts, Card::Rank::Two);
    EXPECT_EQ(twoOfHearts.getValue(), 2);

    Card aceOfSpades(Card::Suit::Spades, Card::Rank::Ace);
    EXPECT_EQ(aceOfSpades.getValue(), 14);
}

TEST_F(CardTest, GetSuitChar)
{
    Card diamondsCard(Card::Suit::Diamonds, Card::Rank::Five);
    EXPECT_EQ(diamondsCard.getSuitChar(), 'd');

    Card heartsCard(Card::Suit::Hearts, Card::Rank::Jack);
    EXPECT_EQ(heartsCard.getSuitChar(), 'h');

    Card spadesCard(Card::Suit::Spades, Card::Rank::King);
    EXPECT_EQ(spadesCard.getSuitChar(), 's');

    Card clubsCard(Card::Suit::Clubs, Card::Rank::Ace);
    EXPECT_EQ(clubsCard.getSuitChar(), 'c');
}

TEST_F(CardTest, GetRankChar)
{
    Card twoCard(Card::Suit::Hearts, Card::Rank::Two);
    EXPECT_EQ(twoCard.getRankChar(), '2');

    Card tenCard(Card::Suit::Spades, Card::Rank::Ten);
    EXPECT_EQ(tenCard.getRankChar(), 'T');

    Card jackCard(Card::Suit::Diamonds, Card::Rank::Jack);
    EXPECT_EQ(jackCard.getRankChar(), 'J');

    Card aceCard(Card::Suit::Clubs, Card::Rank::Ace);
    EXPECT_EQ(aceCard.getRankChar(), 'A');
}

TEST_F(CardTest, ImplicitConversionToInt)
{
    Card card(25);        // Ah
    int cardIndex = card; // Implicit conversion
    EXPECT_EQ(cardIndex, 25);
}

TEST_F(CardTest, EqualityOperators)
{
    Card card1(Card::Suit::Hearts, Card::Rank::Ace);
    Card card2(Card::Suit::Hearts, Card::Rank::Ace);
    Card card3(Card::Suit::Spades, Card::Rank::Ace);

    EXPECT_TRUE(card1 == card2);
    EXPECT_FALSE(card1 == card3);
    EXPECT_FALSE(card1 != card2);
    EXPECT_TRUE(card1 != card3);
}

TEST_F(CardTest, LessThanOperator)
{
    Card card1(0); // 2d
    Card card2(1); // 3d

    EXPECT_TRUE(card1 < card2);
    EXPECT_FALSE(card2 < card1);
}

TEST_F(CardTest, FromIndexStaticMethod)
{
    Card card = Card::fromIndex(51); // Ac - last card
    EXPECT_TRUE(card.isValid());
    EXPECT_EQ(card.getIndex(), 51);
    EXPECT_EQ(card.getSuit(), Card::Suit::Clubs);
    EXPECT_EQ(card.getRank(), Card::Rank::Ace);
}

TEST_F(CardTest, BackwardCompatibilityWithExistingSystem)
{
    // Test that our Card class produces the same results as CardUtilities
    for (int i = 0; i < 52; ++i)
    {
        Card card(i);

        // String representation should match
        EXPECT_EQ(card.toString(), CardUtilities::getCardString(i));

        // Value should match
        EXPECT_EQ(card.getValue(), CardUtilities::getCardValue(CardUtilities::getCardString(i)));

        // Round-trip conversion should work
        Card fromString(CardUtilities::getCardString(i));
        EXPECT_EQ(fromString.getIndex(), i);
    }
}

TEST_F(CardTest, AllSuitsAndRanks)
{
    // Test all combinations to ensure index mapping is correct
    int expectedIndex = 0;

    for (auto suit : {Card::Suit::Diamonds, Card::Suit::Hearts, Card::Suit::Spades, Card::Suit::Clubs})
    {
        for (int rankVal = 2; rankVal <= 14; ++rankVal)
        {
            Card::Rank rank = static_cast<Card::Rank>(rankVal);
            Card card(suit, rank);

            EXPECT_EQ(card.getIndex(), expectedIndex);
            EXPECT_EQ(card.getSuit(), suit);
            EXPECT_EQ(card.getRank(), rank);

            expectedIndex++;
        }
    }
}