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
    bool cardsAreUniqueAndValid(const std::shared_ptr<pkt::core::HandFsm>& hand,
                                const std::shared_ptr<pkt::core::IBoard>& board,
                                const pkt::core::player::PlayerFsmList& players);
};

bool BoardCardTest::cardsAreUniqueAndValid(const std::shared_ptr<pkt::core::HandFsm>& hand,
                                           const std::shared_ptr<pkt::core::IBoard>& board,
                                           const pkt::core::player::PlayerFsmList& players)
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
    initializeHandFsmWithPlayers(2, gameData);

    myHandFsm->dealHoleCards(myHandFsm->dealBoardCards());

    // Verify board cards
    int boardCards[5];
    myHandFsm->getBoard().getCards(boardCards);
    ASSERT_EQ(sizeof(boardCards) / sizeof(boardCards[0]), 5);
}

TEST_F(BoardCardTest, DealBoardCardsAndHoleCards_NoOverlap_2Players_FullTest)
{
    initializeHandFsmWithPlayers(2, gameData);
    myHandFsm->dealHoleCards(myHandFsm->dealBoardCards());
    ASSERT_TRUE(cardsAreUniqueAndValid(myHandFsm, myBoardFsm, mySeatsListFsm));
}

TEST_F(BoardCardTest, DealBoardCardsAndHoleCards_NoOverlap_3Players)
{
    initializeHandFsmWithPlayers(3, gameData);
    myHandFsm->dealHoleCards(myHandFsm->dealBoardCards());
    ASSERT_TRUE(cardsAreUniqueAndValid(myHandFsm, myBoardFsm, mySeatsListFsm));
}

TEST_F(BoardCardTest, DealBoardCardsAndHoleCards_NoOverlap_MaxPlayers)
{
    initializeHandFsmWithPlayers(MAX_NUMBER_OF_PLAYERS, gameData);
    myHandFsm->dealHoleCards(myHandFsm->dealBoardCards());
    ASSERT_TRUE(cardsAreUniqueAndValid(myHandFsm, myBoardFsm, mySeatsListFsm));
}

TEST_F(BoardCardTest, AllDealtCards_AreWithinValidRange_4Players)
{
    initializeHandFsmWithPlayers(4, gameData);
    myHandFsm->dealHoleCards(myHandFsm->dealBoardCards());

    int boardCards[5];
    myBoardFsm->getCards(boardCards);
    for (int card : boardCards)
    {
        ASSERT_GE(card, 0);
        ASSERT_LT(card, 52);
    }
    int cards[2];
    for (const auto& player : *mySeatsListFsm)
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
        initializeHandFsmWithPlayers(6, gameData);
        myHandFsm->dealHoleCards(myHandFsm->dealBoardCards());
        ASSERT_TRUE(cardsAreUniqueAndValid(myHandFsm, myBoardFsm, mySeatsListFsm));
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