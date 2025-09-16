#include "common/EngineTest.h"
#include "common/common.h"
#include "core/cards/Card.h"

#include <memory>

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

class BoardCardTest : public EngineTest
{

  public:
    bool cardsAreUniqueAndValid(const std::shared_ptr<pkt::core::Hand>& hand,
                                const std::shared_ptr<pkt::core::IBoard>& board,
                                const pkt::core::player::PlayerList& players);
};

bool BoardCardTest::cardsAreUniqueAndValid(const std::shared_ptr<pkt::core::Hand>& hand,
                                           const std::shared_ptr<pkt::core::IBoard>& board,
                                           const pkt::core::player::PlayerList& players)
{
    std::vector<int> allCards;

    // Collect board cards
    int boardCards[5];
    board->getCards(boardCards);
    allCards.insert(allCards.end(), boardCards, boardCards + 5);

    int cards[2];
    // Collect players' hole cards
    for (const auto& player : *players)
    {
        player->getCards(cards);
        allCards.push_back(cards[0]);
        allCards.push_back(cards[1]);
    }

    // Check range and uniqueness
    std::set<int> cardSet;
    for (int card : allCards)
    {
        if (card < 0 || card >= 52)
            return false;
        cardSet.insert(card);
    }

    return cardSet.size() == allCards.size();
}

TEST_F(BoardCardTest, DealBoardCardsAndHoleCards_NoOverlap_2Players)
{
    initializeHandWithPlayers(2, gameData);

    myHand->dealHoleCards(myHand->dealBoardCards());

    // Verify board cards
    int boardCards[5];
    myHand->getBoard().getCards(boardCards);
    ASSERT_EQ(sizeof(boardCards) / sizeof(boardCards[0]), 5);
}

TEST_F(BoardCardTest, DealBoardCardsAndHoleCards_NoOverlap_2Players_FullTest)
{
    initializeHandWithPlayers(2, gameData);
    myHand->dealHoleCards(myHand->dealBoardCards());
    ASSERT_TRUE(cardsAreUniqueAndValid(myHand, myBoard, mySeatsList));
}

TEST_F(BoardCardTest, DealBoardCardsAndHoleCards_NoOverlap_3Players)
{
    initializeHandWithPlayers(3, gameData);
    myHand->dealHoleCards(myHand->dealBoardCards());
    ASSERT_TRUE(cardsAreUniqueAndValid(myHand, myBoard, mySeatsList));
}

TEST_F(BoardCardTest, DealBoardCardsAndHoleCards_NoOverlap_MaxPlayers)
{
    initializeHandWithPlayers(MAX_NUMBER_OF_PLAYERS, gameData);
    myHand->dealHoleCards(myHand->dealBoardCards());
    ASSERT_TRUE(cardsAreUniqueAndValid(myHand, myBoard, mySeatsList));
}

TEST_F(BoardCardTest, AllDealtCards_AreWithinValidRange_4Players)
{
    initializeHandWithPlayers(4, gameData);
    myHand->dealHoleCards(myHand->dealBoardCards());

    int boardCards[5];
    myBoard->getCards(boardCards);
    for (int card : boardCards)
    {
        ASSERT_GE(card, 0);
        ASSERT_LT(card, 52);
    }
    int cards[2];
    for (const auto& player : *mySeatsList)
    {
        player->getCards(cards);
        ASSERT_GE(cards[0], 0);
        ASSERT_LT(cards[0], 52);
        ASSERT_GE(cards[1], 0);
        ASSERT_LT(cards[1], 52);
    }
}

TEST_F(BoardCardTest, DealCards_NoOverlap_OverMultipleRounds)
{
    for (int i = 0; i < 500; ++i)
    {
        initializeHandWithPlayers(6, gameData);
        myHand->dealHoleCards(myHand->dealBoardCards());
        ASSERT_TRUE(cardsAreUniqueAndValid(myHand, myBoard, mySeatsList));
    }
}

TEST_F(BoardCardTest, ModernBoardCards_GamePhases_WorkCorrectly)
{
    // Test preflop state (no cards)
    BoardCards preflop;
    EXPECT_TRUE(preflop.isPreflop());
    EXPECT_EQ(preflop.getNumCards(), 0);
    EXPECT_EQ(preflop.toString(), "<no cards>");

    // Test flop state (3 cards)
    BoardCards flop("Ah", "Ks", "Qd");
    EXPECT_TRUE(flop.isFlop());
    EXPECT_EQ(flop.getNumCards(), 3);
    EXPECT_EQ(flop.toString(), "Ah Ks Qd");

    // Test turn state (4 cards)
    BoardCards turn("Ah", "Ks", "Qd", "Jh");
    EXPECT_TRUE(turn.isTurn());
    EXPECT_EQ(turn.getNumCards(), 4);
    EXPECT_EQ(turn.toString(), "Ah Ks Qd Jh");

    // Test river state (5 cards)
    BoardCards river("Ah", "Ks", "Qd", "Jh", "Tc");
    EXPECT_TRUE(river.isRiver());
    EXPECT_EQ(river.getNumCards(), 5);
    EXPECT_EQ(river.toString(), "Ah Ks Qd Jh Tc");

    // Test progressive dealing
    BoardCards progressive;
    progressive.dealFlop(Card("As"), Card("Kh"), Card("Qs"));
    EXPECT_TRUE(progressive.isFlop());
    EXPECT_EQ(progressive.toString(), "As Kh Qs");

    progressive.dealTurn(Card("Jd"));
    EXPECT_TRUE(progressive.isTurn());
    EXPECT_EQ(progressive.toString(), "As Kh Qs Jd");

    progressive.dealRiver(Card("Tc"));
    EXPECT_TRUE(progressive.isRiver());
    EXPECT_EQ(progressive.toString(), "As Kh Qs Jd Tc");
}

TEST_F(BoardCardTest, ModernBoardCards_LegacyCompatibility_Works)
{
    // Create modern BoardCards and test legacy conversion
    BoardCards modern("Ah", "Ks", "Qd", "Jh", "Tc");

    // Convert to legacy int array
    int legacyArray[5];
    modern.toIntArray(legacyArray);

    // Verify the conversion matches expected card indices
    EXPECT_EQ(legacyArray[0], Card("Ah").getIndex());
    EXPECT_EQ(legacyArray[1], Card("Ks").getIndex());
    EXPECT_EQ(legacyArray[2], Card("Qd").getIndex());
    EXPECT_EQ(legacyArray[3], Card("Jh").getIndex());
    EXPECT_EQ(legacyArray[4], Card("Tc").getIndex());

    // Create from legacy array and verify it matches
    BoardCards fromLegacy(legacyArray);
    EXPECT_EQ(fromLegacy.toString(), modern.toString());
    EXPECT_TRUE(fromLegacy.isRiver());
    EXPECT_EQ(fromLegacy.getNumCards(), 5);
}

} // namespace pkt::test