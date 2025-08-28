#include "common/common.h"

#include "CardsTest.h"

#include <memory>

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

bool CardsTest::cardsAreUniqueAndValid(const std::shared_ptr<pkt::core::IHand>& hand,
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

TEST_F(CardsTest, DealBoardCardsAndHoleCards_NoOverlap_2Players)
{
    initializeHandForTesting(2, gameData);

    myHand->dealHoleCards(myHand->dealBoardCards());

    // Verify board cards
    int boardCards[5];
    myHand->getBoard()->getCards(boardCards);
    ASSERT_EQ(sizeof(boardCards) / sizeof(boardCards[0]), 5);
}

TEST_F(CardsTest, DealBoardCardsAndHoleCards_NoOverlap_2Players_FullTest)
{
    initializeHandForTesting(2, gameData);
    myHand->dealHoleCards(myHand->dealBoardCards());
    ASSERT_TRUE(cardsAreUniqueAndValid(myHand, myBoard, mySeatsList));
}

TEST_F(CardsTest, DealBoardCardsAndHoleCards_NoOverlap_3Players)
{
    initializeHandForTesting(3, gameData);
    myHand->dealHoleCards(myHand->dealBoardCards());
    ASSERT_TRUE(cardsAreUniqueAndValid(myHand, myBoard, mySeatsList));
}

TEST_F(CardsTest, DealBoardCardsAndHoleCards_NoOverlap_MaxPlayers)
{
    initializeHandForTesting(MAX_NUMBER_OF_PLAYERS, gameData);
    myHand->dealHoleCards(myHand->dealBoardCards());
    ASSERT_TRUE(cardsAreUniqueAndValid(myHand, myBoard, mySeatsList));
}

TEST_F(CardsTest, AllDealtCards_AreWithinValidRange_4Players)
{
    initializeHandForTesting(4, gameData);
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

TEST_F(CardsTest, DealCards_NoOverlap_OverMultipleRounds)
{
    for (int i = 0; i < 500; ++i)
    {
        initializeHandForTesting(6, gameData);
        myHand->dealHoleCards(myHand->dealBoardCards());
        ASSERT_TRUE(cardsAreUniqueAndValid(myHand, myBoard, mySeatsList));
    }
}

} // namespace pkt::test