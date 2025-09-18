#include <gtest/gtest.h>
#include <vector>
#include "core/engine/cards/Card.h"
#include "core/engine/cards/CardUtilities.h"

using namespace pkt::core;

class CardCompatibilityTest : public ::testing::Test
{
  protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(CardCompatibilityTest, ExistingIntArraysWithNewCardClass)
{
    // Simulate existing player cards as int array (current system)
    int playerCards[2] = {25, 37}; // Ah, Ks

    // Convert to Card objects for new event system
    Card card1 = Card::fromIndex(playerCards[0]);
    Card card2 = Card::fromIndex(playerCards[1]);

    // Verify the cards are correct
    EXPECT_EQ(card1.toString(), "Ah");
    EXPECT_EQ(card2.toString(), "Ks");

    // Verify we can still get int values for existing code
    EXPECT_EQ(static_cast<int>(card1), 25);
    EXPECT_EQ(static_cast<int>(card2), 37);

    // Verify existing CardUtilities still work
    EXPECT_EQ(CardUtilities::getCardString(playerCards[0]), "Ah");
    EXPECT_EQ(CardUtilities::getCardString(playerCards[1]), "Ks");
}

TEST_F(CardCompatibilityTest, BoardCardsCompatibility)
{
    // Simulate existing board cards as int array (current system)
    int boardCards[5] = {12, 24, 36, 48, 51}; // Ad, Kh, Qs, Jc, Ac

    // Convert to Card objects for new events
    std::vector<Card> cards;
    for (int i = 0; i < 5; ++i)
    {
        cards.push_back(Card::fromIndex(boardCards[i]));
    }

    // Verify all cards
    EXPECT_EQ(cards[0].toString(), "Ad");
    EXPECT_EQ(cards[1].toString(), "Kh");
    EXPECT_EQ(cards[2].toString(), "Qs");
    EXPECT_EQ(cards[3].toString(), "Jc");
    EXPECT_EQ(cards[4].toString(), "Ac");

    // Verify we can convert back to int array for existing code
    int convertedBack[5];
    for (int i = 0; i < 5; ++i)
    {
        convertedBack[i] = cards[i].getIndex();
    }

    // Should match original array
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(convertedBack[i], boardCards[i]);
    }
}

TEST_F(CardCompatibilityTest, CreateCardsFromStringAndUseWithExistingCode)
{
    // Create cards using new string interface
    Card aceOfSpades("As");
    Card kingOfHearts("Kh");

    // Use with existing int-based functions
    int aceIndex = aceOfSpades.getIndex();
    int kingIndex = kingOfHearts.getIndex();

    // Verify CardUtilities works with our indices
    EXPECT_EQ(CardUtilities::getCardString(aceIndex), "As");
    EXPECT_EQ(CardUtilities::getCardString(kingIndex), "Kh");

    // Verify values for game logic
    EXPECT_EQ(CardUtilities::getCardValue("As"), 14);
    EXPECT_EQ(CardUtilities::getCardValue("Kh"), 13);
}

TEST_F(CardCompatibilityTest, HandEvaluationCompatibility)
{
    // Create a poker hand using Card objects
    std::vector<Card> hand = {Card("As"), Card("Ks"), Card("Qs"), Card("Js"), Card("Ts")};

    // Convert to int array for existing hand evaluation code
    int handArray[5];
    for (size_t i = 0; i < hand.size(); ++i)
    {
        handArray[i] = hand[i].getIndex();
    }

    // Verify we have a royal flush in spades
    EXPECT_EQ(CardUtilities::getCardString(handArray[0]), "As");
    EXPECT_EQ(CardUtilities::getCardString(handArray[1]), "Ks");
    EXPECT_EQ(CardUtilities::getCardString(handArray[2]), "Qs");
    EXPECT_EQ(CardUtilities::getCardString(handArray[3]), "Js");
    EXPECT_EQ(CardUtilities::getCardString(handArray[4]), "Ts");

    // All should be spades
    for (const auto& card : hand)
    {
        EXPECT_EQ(card.getSuit(), Card::Suit::Spades);
    }
}