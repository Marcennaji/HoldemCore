#include "core/engine/cards/DeckManager.h"
#include "common/FakeRandomizer.h"
#include <gtest/gtest.h>
#include <memory>

namespace pkt::core
{

class DeckManagerTest : public ::testing::Test
{
  protected:
    std::shared_ptr<test::FakeRandomizer> randomizer = std::make_shared<test::FakeRandomizer>();
    DeckManager deckManager{randomizer};
};

TEST_F(DeckManagerTest, InitializeAndShuffleCreatesValidDeck)
{
    deckManager.initializeAndShuffle();

    EXPECT_EQ(52, deckManager.remainingCards());
    EXPECT_TRUE(deckManager.hasEnoughCards(5, 9)); // 5 board + 9 players * 2 = 23 cards needed
}

TEST_F(DeckManagerTest, DealCardsReducesRemainingCards)
{
    deckManager.initializeAndShuffle();

    auto cards = deckManager.dealCards(5);

    EXPECT_EQ(5, cards.size());
    EXPECT_EQ(47, deckManager.remainingCards());
}

TEST_F(DeckManagerTest, DealCardsThrowsWhenNotEnoughCards)
{
    deckManager.initializeAndShuffle();
    deckManager.dealCards(50); // Leave only 2 cards

    EXPECT_THROW(deckManager.dealCards(5), std::runtime_error);
}

TEST_F(DeckManagerTest, HasEnoughCardsCalculatesCorrectly)
{
    deckManager.initializeAndShuffle();

    EXPECT_TRUE(deckManager.hasEnoughCards(5, 9));   // 23 cards needed, 52 available
    EXPECT_TRUE(deckManager.hasEnoughCards(0, 26));  // 52 cards needed, 52 available
    EXPECT_FALSE(deckManager.hasEnoughCards(5, 24)); // 53 cards needed, 52 available
}

TEST_F(DeckManagerTest, ResetDealPositionRestoresFullDeck)
{
    deckManager.initializeAndShuffle();
    deckManager.dealCards(10);
    EXPECT_EQ(42, deckManager.remainingCards());

    deckManager.resetDealPosition();

    EXPECT_EQ(52, deckManager.remainingCards());
}

} // namespace pkt::core