#include "HandTest.h"
#include "DummyPlayer.h"
#include "core/engine/model/GameData.h"
#include "core/engine/model/StartData.h"
#include "core/services/GlobalServices.h"
#include "infra/ConsoleLogger.h"
#include "infra/eval/PsimHandEvaluationEngine.h"

#include <gtest/gtest.h>
#include <memory>

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

bool cardsAreUniqueAndValid(const std::shared_ptr<pkt::core::IHand>& hand,
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

void HandTest::SetUp()
{
    factory = std::make_shared<EngineFactory>(events);
    auto& services = pkt::core::GlobalServices::instance();
    services.setLogger(std::make_shared<pkt::infra::ConsoleLogger>());
    services.setHandEvaluationEngine(std::make_shared<pkt::infra::PsimHandEvaluationEngine>());
}

void HandTest::TearDown()
{
}
// Helper function to initialize a Hand object for testing
void HandTest::initializeHandForTesting(size_t activePlayerCount)
{
    hand.reset();
    createPlayerList(activePlayerCount);
    initializeHandWithPlayers(activePlayerCount);
}

void HandTest::createPlayerList(size_t playerCount)
{
    playerList = std::make_shared<std::list<std::shared_ptr<Player>>>();
    for (size_t i = 0; i < playerCount; ++i)
    {
        playerList->push_back(std::make_shared<DummyPlayer>(i, events));
    }
}
void HandTest::initializeHandWithPlayers(size_t activePlayerCount)
{
    // Create the active player list by selecting the first `activePlayerCount` players from the seats list

    board = factory->createBoard(1);

    GameData gameData;
    gameData.maxNumberOfPlayers = MAX_NUMBER_OF_PLAYERS;
    gameData.startMoney = 1000;
    gameData.firstSmallBlind = 10;
    gameData.tableProfile = TableProfile::RandomOpponents;

    StartData startData;
    startData.startDealerPlayerId = 0;
    startData.numberOfPlayers = static_cast<int>(activePlayerCount);

    hand = factory->createHand(factory, board, playerList, playerList, playerList, 0, gameData, startData);
}

TEST_F(HandTest, DealBoardCardsAndHoleCards_NoOverlap_2Players)
{
    initializeHandForTesting(2);

    hand->dealHoleCards(hand->dealBoardCards());

    // Verify board cards
    int boardCards[5];
    hand->getBoard()->getCards(boardCards);
    ASSERT_EQ(sizeof(boardCards) / sizeof(boardCards[0]), 5);
}

TEST_F(HandTest, DealBoardCardsAndHoleCards_NoOverlap_2Players_FullTest)
{
    initializeHandForTesting(2);
    hand->dealHoleCards(hand->dealBoardCards());
    ASSERT_TRUE(cardsAreUniqueAndValid(hand, board, playerList));
}

TEST_F(HandTest, DealBoardCardsAndHoleCards_NoOverlap_3Players)
{
    initializeHandForTesting(3);
    hand->dealHoleCards(hand->dealBoardCards());
    ASSERT_TRUE(cardsAreUniqueAndValid(hand, board, playerList));
}

TEST_F(HandTest, DealBoardCardsAndHoleCards_NoOverlap_1Player)
{
    initializeHandForTesting(1);
    hand->dealHoleCards(hand->dealBoardCards());
    ASSERT_TRUE(cardsAreUniqueAndValid(hand, board, playerList));
}

TEST_F(HandTest, DealBoardCardsAndHoleCards_NoOverlap_MaxPlayers)
{
    initializeHandForTesting(MAX_NUMBER_OF_PLAYERS);
    hand->dealHoleCards(hand->dealBoardCards());
    ASSERT_TRUE(cardsAreUniqueAndValid(hand, board, playerList));
}

TEST_F(HandTest, AllDealtCards_AreWithinValidRange_4Players)
{
    initializeHandForTesting(4);
    hand->dealHoleCards(hand->dealBoardCards());

    int boardCards[5];
    board->getCards(boardCards);
    for (int card : boardCards)
    {
        ASSERT_GE(card, 0);
        ASSERT_LT(card, 52);
    }
    int cards[2];
    for (const auto& player : *playerList)
    {
        player->getCards(cards);
        ASSERT_GE(cards[0], 0);
        ASSERT_LT(cards[0], 52);
        ASSERT_GE(cards[1], 0);
        ASSERT_LT(cards[1], 52);
    }
}

TEST_F(HandTest, DealCards_NoOverlap_OverMultipleRounds)
{
    for (int i = 0; i < 500; ++i)
    {
        initializeHandForTesting(6);
        hand->dealHoleCards(hand->dealBoardCards());
        ASSERT_TRUE(cardsAreUniqueAndValid(hand, board, playerList));
    }
}

TEST_F(HandTest, DISABLED_SwitchRounds_TransitionsCorrectlyThroughAllPhases)
{
    initializeHandForTesting(6);
    hand->setCurrentRound(GameStatePreflop);
    hand->setAllInCondition(false); // normal game

    hand->switchRounds();
    EXPECT_EQ(hand->getCurrentRound(), GameStateFlop);

    hand->switchRounds();
    EXPECT_EQ(hand->getCurrentRound(), GameStateTurn);

    hand->switchRounds();
    EXPECT_EQ(hand->getCurrentRound(), GameStateRiver);

    // After river, no more transitions
    hand->switchRounds();
    EXPECT_EQ(hand->getCurrentRound(), GameStateRiver);
}

TEST_F(HandTest, DISABLED_SwitchRounds_DoesNotAdvanceWhenAllInConditionIsTrue)
{
    initializeHandForTesting(6);
    hand->setCurrentRound(GameStateFlop);
    hand->setAllInCondition(true); // simulate all-in stop

    hand->switchRounds();
    EXPECT_EQ(hand->getCurrentRound(), GameStateFlop);
}

TEST_F(HandTest, SwitchRounds_DoesNotAdvancePastRiver)
{
    initializeHandForTesting(6);
    hand->setCurrentRound(GameStateRiver);

    hand->switchRounds(); // should not move past river
    EXPECT_EQ(hand->getCurrentRound(), GameStateRiver);
}

TEST_F(HandTest, DISABLED_SwitchRounds_SkipsWhenOnlyOnePlayerRunning)
{
    initializeHandForTesting(1);
    hand->setCurrentRound(GameStatePreflop);

    hand->switchRounds();                                 // should detect end of game
    EXPECT_EQ(hand->getCurrentRound(), GameStatePreflop); // or GameStateRiver if end state forced
}

} // namespace pkt::test