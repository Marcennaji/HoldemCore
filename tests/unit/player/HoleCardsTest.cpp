#include <gtest/gtest.h>
#include "core/engine/GameEvents.h"
#include "core/engine/cards/Card.h"
#include "infra/persistence/NullPlayersStatisticsStore.h"
#include "core/player/Player.h"
#include "core/services/DefaultRandomizer.h"
#include "common/DummyPlayer.h"
#include "infra/ConsoleLogger.h"
#include "infra/eval/PsimHandEvaluationEngine.h"

using namespace pkt::core;
using namespace pkt::core::player;

class HoleCardsTest : public ::testing::Test
{
  protected:
    void SetUp() override {
        auto logger = std::make_shared<pkt::infra::ConsoleLogger>();
        logger->setLogLevel(pkt::core::LogLevel::Info);
        m_logger = logger;
        m_handEvaluator = std::make_shared<pkt::infra::PsimHandEvaluationEngine>();
        m_statisticsStore = std::make_shared<pkt::core::NullPlayersStatisticsStore>();
        m_randomizer = std::make_shared<pkt::core::DefaultRandomizer>();
    }
    void TearDown() override {}
    
    std::shared_ptr<pkt::core::Logger> m_logger;
    std::shared_ptr<pkt::core::HandEvaluationEngine> m_handEvaluator;
    std::shared_ptr<pkt::core::PlayersStatisticsStore> m_statisticsStore;
    std::shared_ptr<pkt::core::Randomizer> m_randomizer;
    GameEvents m_events;
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

TEST_F(HoleCardsTest, PlayerModernInterface)
{
    auto player = std::make_shared<pkt::test::DummyPlayer>(1, m_events, m_logger, m_handEvaluator, m_statisticsStore, m_randomizer);

    // Test setting cards using new Card-based interface (preferred)
    player->setHoleCards(Card("Qd"), Card("Jc"));

    // Test getting cards using new interface
    const HoleCards& retrievedCards = player->getHoleCards();

    EXPECT_TRUE(retrievedCards.isValid());
    EXPECT_EQ(retrievedCards.card1.toString(), "Qd");
    EXPECT_EQ(retrievedCards.card2.toString(), "Jc");
    EXPECT_EQ(retrievedCards.toString(), "Qd Jc");
}

TEST_F(HoleCardsTest, PlayerStringConstructorInterface)
{
    auto player = std::make_shared<pkt::test::DummyPlayer>(1, m_events, m_logger, m_handEvaluator, m_statisticsStore, m_randomizer);

    // Test setting cards using string-based HoleCards constructor
    HoleCards holeCards("As", "Kh");
    player->setHoleCards(holeCards);

    // Verify the cards are set correctly
    const HoleCards& retrieved = player->getHoleCards();
    EXPECT_TRUE(retrieved.isValid());
    EXPECT_EQ(retrieved.card1.toString(), "As");
    EXPECT_EQ(retrieved.card2.toString(), "Kh");
    EXPECT_EQ(player->getCardsValueString(), "As Kh");
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

TEST_F(HoleCardsTest, ApiTest)
{
    auto player = std::make_shared<pkt::test::DummyPlayer>(1, m_events, m_logger, m_handEvaluator, m_statisticsStore, m_randomizer);

    // NEW APPROACH: Work directly with Card objects
    Card aceOfSpades("As");
    Card kingOfHearts("Kh");

    // Set cards using modern interface - preferred for new FSM code
    player->setHoleCards(aceOfSpades, kingOfHearts);

    // Access cards as Card objects - type-safe and expressive
    const HoleCards& holeCards = player->getHoleCards();

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

TEST_F(HoleCardsTest, ConvenientStringConstructor)
{
    auto player = std::make_shared<pkt::test::DummyPlayer>(2, m_events, m_logger, m_handEvaluator, m_statisticsStore, m_randomizer);

    // CONVENIENT: Create from strings directly
    HoleCards pocket("Qd", "Jc");
    player->setHoleCards(pocket);

    const HoleCards& retrieved = player->getHoleCards();
    EXPECT_EQ(retrieved.card1.toString(), "Qd");
    EXPECT_EQ(retrieved.card2.toString(), "Jc");

    // Access suit/rank information easily
    EXPECT_EQ(retrieved.card1.getSuit(), Card::Suit::Diamonds);
    EXPECT_EQ(retrieved.card2.getSuit(), Card::Suit::Clubs);
    EXPECT_EQ(retrieved.card1.getRank(), Card::Rank::Queen);
    EXPECT_EQ(retrieved.card2.getRank(), Card::Rank::Jack);
}