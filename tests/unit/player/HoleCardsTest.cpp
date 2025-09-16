#include <gtest/gtest.h>
#include "core/cards/Card.h"
#include "core/engine/GameEvents.h"
#include "core/player/Player.h"

using namespace pkt::core;
using namespace pkt::core::player;

class HoleCardsTest : public ::testing::Test
{
  protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(HoleCardsTest, DefaultConstructorCreatesInvalidCards)
{
    HoleCards holeCards;
    EXPECT_FALSE(holeCards.isValid());
    EXPECT_EQ(holeCards.toString(), "Invalid Cards");
}

TEST_F(HoleCardsTest, ConstructorFromCardObjects)
{
    Card aceOfSpades("As");
    Card kingOfHearts("Kh");

    HoleCards holeCards(aceOfSpades, kingOfHearts);

    EXPECT_TRUE(holeCards.isValid());
    EXPECT_EQ(holeCards.card1.toString(), "As");
    EXPECT_EQ(holeCards.card2.toString(), "Kh");
    EXPECT_EQ(holeCards.toString(), "As Kh");
}

TEST_F(HoleCardsTest, ConstructorFromCardStrings)
{
    HoleCards holeCards("Qd", "Jc");

    EXPECT_TRUE(holeCards.isValid());
    EXPECT_EQ(holeCards.card1.toString(), "Qd");
    EXPECT_EQ(holeCards.card2.toString(), "Jc");
    EXPECT_EQ(holeCards.toString(), "Qd Jc");
}

TEST_F(HoleCardsTest, ConstructorFromIntArray)
{
    int cardIndices[2] = {38, 24}; // As, Kh
    HoleCards holeCards(cardIndices);

    EXPECT_TRUE(holeCards.isValid());
    EXPECT_EQ(holeCards.card1.toString(), "As");
    EXPECT_EQ(holeCards.card2.toString(), "Kh");
    EXPECT_EQ(holeCards.toString(), "As Kh");
}

TEST_F(HoleCardsTest, ToIntArrayConversion)
{
    HoleCards holeCards(Card("Qd"), Card("Jc"));
    int cardIndices[2];

    holeCards.toIntArray(cardIndices);

    EXPECT_EQ(cardIndices[0], 10); // Qd
    EXPECT_EQ(cardIndices[1], 48); // Jc
}

TEST_F(HoleCardsTest, FromIntArrayConversion)
{
    HoleCards holeCards;
    int cardIndices[2] = {25, 37}; // Ah, Ks

    holeCards.fromIntArray(cardIndices);

    EXPECT_TRUE(holeCards.isValid());
    EXPECT_EQ(holeCards.card1.toString(), "Ah");
    EXPECT_EQ(holeCards.card2.toString(), "Ks");
}

TEST_F(HoleCardsTest, ResetToInvalidCards)
{
    HoleCards holeCards(Card("As"), Card("Kh"));
    EXPECT_TRUE(holeCards.isValid());

    holeCards.reset();
    EXPECT_FALSE(holeCards.isValid());
}

TEST_F(HoleCardsTest, PlayerBackwardCompatibility)
{
    GameEvents events;
    Player player(events, 1, "TestPlayer", 1000);

    // Test setting cards using old int array method
    int cardIndices[2] = {25, 37}; // Ah, Ks
    player.setCards(cardIndices);

    // Test getting cards using old int array method
    int retrievedCards[2];
    player.getCards(retrievedCards);

    EXPECT_EQ(retrievedCards[0], 25);
    EXPECT_EQ(retrievedCards[1], 37);

    // Test that the Card-first approach works
    const HoleCards& holeCards = player.getHoleCards();
    EXPECT_EQ(holeCards.card1.toString(), "Ah");
    EXPECT_EQ(holeCards.card2.toString(), "Ks");
    EXPECT_EQ(player.getCardsValueString(), "Ah Ks");
}

TEST_F(HoleCardsTest, PlayerModernInterface)
{
    GameEvents events;
    Player player(events, 1, "TestPlayer", 1000);

    // Test setting cards using new Card-based interface (preferred)
    player.setHoleCards(Card("Qd"), Card("Jc"));

    // Test getting cards using new interface
    const HoleCards& retrievedCards = player.getHoleCards();

    EXPECT_TRUE(retrievedCards.isValid());
    EXPECT_EQ(retrievedCards.card1.toString(), "Qd");
    EXPECT_EQ(retrievedCards.card2.toString(), "Jc");
    EXPECT_EQ(retrievedCards.toString(), "Qd Jc");

    // Test that legacy interface still works for existing engine integration
    int cardIndices[2];
    player.getCards(cardIndices);
    EXPECT_EQ(cardIndices[0], 10); // Qd
    EXPECT_EQ(cardIndices[1], 48); // Jc
}

TEST_F(HoleCardsTest, PlayerStringConstructorInterface)
{
    GameEvents events;
    Player player(events, 1, "TestPlayer", 1000);

    // Test setting cards using string-based HoleCards constructor
    HoleCards holeCards("As", "Kh");
    player.setHoleCards(holeCards);

    // Verify the cards are set correctly
    const HoleCards& retrieved = player.getHoleCards();
    EXPECT_TRUE(retrieved.isValid());
    EXPECT_EQ(retrieved.card1.toString(), "As");
    EXPECT_EQ(retrieved.card2.toString(), "Kh");
    EXPECT_EQ(player.getCardsValueString(), "As Kh");
}

TEST_F(HoleCardsTest, RoundTripConversion)
{
    // Test that converting back and forth preserves the data
    HoleCards original(Card("Tc"), Card("9h"));

    // Convert to int array
    int cardIndices[2];
    original.toIntArray(cardIndices);

    // Convert back to HoleCards
    HoleCards converted;
    converted.fromIntArray(cardIndices);

    // Should be identical
    EXPECT_EQ(original.card1.toString(), converted.card1.toString());
    EXPECT_EQ(original.card2.toString(), converted.card2.toString());
    EXPECT_EQ(original.toString(), converted.toString());
}