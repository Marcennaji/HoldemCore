#include <gtest/gtest.h>
#include "core/cards/Card.h"
#include "core/engine/GameEvents.h"
#include "core/player/Player.h"

using namespace pkt::core;
using namespace pkt::core::player;

class CardFirstApproachTest : public ::testing::Test
{
  protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(CardFirstApproachTest, ModernCardFirstWorkflow)
{
    GameEvents events;
    Player player(events, 1, "ModernPlayer", 1000);

    // NEW APPROACH: Work directly with Card objects
    Card aceOfSpades("As");
    Card kingOfHearts("Kh");

    // Set cards using modern interface - preferred for new FSM code
    player.setHoleCards(aceOfSpades, kingOfHearts);

    // Access cards as Card objects - type-safe and expressive
    const HoleCards& holeCards = player.getHoleCards();

    // Rich Card API available
    EXPECT_EQ(holeCards.card1.getSuit(), Card::Suit::Spades);
    EXPECT_EQ(holeCards.card1.getRank(), Card::Rank::Ace);
    EXPECT_EQ(holeCards.card2.getSuit(), Card::Suit::Hearts);
    EXPECT_EQ(holeCards.card2.getRank(), Card::Rank::King);

    // String representation for logging/display
    EXPECT_EQ(holeCards.toString(), "As Kh");

    // Values for game logic
    EXPECT_EQ(holeCards.card1.getValue(), 14); // Ace high
    EXPECT_EQ(holeCards.card2.getValue(), 13); // King
}

TEST_F(CardFirstApproachTest, ConvenientStringConstructor)
{
    GameEvents events;
    Player player(events, 2, "ConvenientPlayer", 1000);

    // CONVENIENT: Create from strings directly
    HoleCards pocket("Qd", "Jc");
    player.setHoleCards(pocket);

    const HoleCards& retrieved = player.getHoleCards();
    EXPECT_EQ(retrieved.card1.toString(), "Qd");
    EXPECT_EQ(retrieved.card2.toString(), "Jc");

    // Access suit/rank information easily
    EXPECT_EQ(retrieved.card1.getSuit(), Card::Suit::Diamonds);
    EXPECT_EQ(retrieved.card2.getSuit(), Card::Suit::Clubs);
    EXPECT_EQ(retrieved.card1.getRank(), Card::Rank::Queen);
    EXPECT_EQ(retrieved.card2.getRank(), Card::Rank::Jack);
}

TEST_F(CardFirstApproachTest, LegacyInterfaceStillWorksForEngine)
{
    GameEvents events;
    Player player(events, 3, "BridgePlayer", 1000);

    // When interfacing with legacy engine code that uses int arrays
    int legacyCards[2] = {51, 0}; // Ac, 2d
    player.setCards(legacyCards);

    // Modern code can still access as Card objects
    const HoleCards& holeCards = player.getHoleCards();
    EXPECT_EQ(holeCards.card1.toString(), "Ac");
    EXPECT_EQ(holeCards.card2.toString(), "2d");

    // Legacy engine can still get int array when needed
    int retrievedCards[2];
    player.getCards(retrievedCards);
    EXPECT_EQ(retrievedCards[0], 51);
    EXPECT_EQ(retrievedCards[1], 0);
}