#include "HandTest.h"
#include "DummyPlayer.h"

#include <gtest/gtest.h>
#include <memory>

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

void HandTest::SetUp()
{
    logger = std::make_shared<pkt::core::NullLogger>();
    factory = std::make_shared<EngineFactory>(events, logger.get());
}

void HandTest::TearDown()
{
}
// Helper function to initialize a Hand object for testing
void HandTest::initializeHandForTesting(size_t activePlayerCount)
{
    hand.reset();
    core::player::PlayerList seatsList = createPlayerList(activePlayerCount);
    initializeHandWithPlayers(seatsList, activePlayerCount);
}

PlayerList HandTest::createPlayerList(size_t playerCount)
{
    auto playerList = std::make_shared<std::list<std::shared_ptr<Player>>>();
    for (size_t i = 0; i < playerCount; ++i)
    {
        playerList->push_back(std::make_shared<DummyPlayer>(i + 1, events));
    }
    return playerList;
}
void HandTest::initializeHandWithPlayers(PlayerList seatsList, size_t activePlayerCount)
{
    // Create the active player list by selecting the first `activePlayerCount` players from the seats list
    PlayerList activePlayerList = std::make_shared<std::list<std::shared_ptr<Player>>>(
        seatsList->begin(), std::next(seatsList->begin(), activePlayerCount));

    board = factory->createBoard(1);
    // Create the Hand object with nullptrs for irrelevant parameters
    hand = factory->createHand(factory, board, nullptr, nullptr, nullptr, seatsList, activePlayerList, activePlayerList,
                               0, static_cast<int>(activePlayerCount), 1, 10, 1000);
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
} // namespace pkt::test