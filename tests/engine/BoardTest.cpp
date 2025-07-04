// tests/BoardTest.cpp
#include "core/engine/Board.h"
#include <gtest/gtest.h>
#include "DummyPlayer.h"

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

class BoardTest : public ::testing::Test
{
  protected:
    std::shared_ptr<Board> board;
    PlayerList seats;
    pkt::core::GameEvents events;

    std::shared_ptr<DummyPlayer> createPlayer(int id, int startCash, int currentCash, int handValue,
                                              PlayerAction action)
    {
        auto p = std::make_shared<DummyPlayer>(id, events);
        p->setId(id);
        p->setRoundStartCash(startCash);
        p->setCash(currentCash);
        p->setHandRanking(handValue);
        p->setAction(action);
        return p;
    }

    void setupBoard(std::initializer_list<std::shared_ptr<DummyPlayer>> playerList, int dealerId, int pot)
    {
        board = std::make_shared<Board>(dealerId);

        seats = std::make_shared<std::list<std::shared_ptr<Player>>>();
        for (const auto& dummyPlayer : playerList)
        {
            seats->push_back(std::static_pointer_cast<Player>(dummyPlayer));
        }
        board->setSeatsList(seats);
        board->setPot(pot);
    }
};

TEST_F(BoardTest, SingleWinnerGetsEntirePot)
{
    auto p1 = createPlayer(0, 500, 0, 100, PlayerActionCall);
    auto p2 = createPlayer(1, 500, 0, 80, PlayerActionCall);
    auto p3 = createPlayer(2, 300, 0, 50, PlayerActionCall);
    setupBoard({p1, p2, p3}, 0, 1300);

    board->distributePot();

    EXPECT_EQ(p1->getCash(), 1300);
    EXPECT_EQ(p2->getCash(), 0);
    EXPECT_EQ(p3->getCash(), 0);
    EXPECT_EQ(board->getWinners().size(), 1);
    EXPECT_EQ(board->getWinners().front(), 0);
}

TEST_F(BoardTest, SplitPotBetweenTwoWinners)
{
    auto p1 = createPlayer(0, 500, 0, 100, PlayerActionCall);
    auto p2 = createPlayer(1, 500, 0, 100, PlayerActionCall);
    auto p3 = createPlayer(2, 300, 0, 50, PlayerActionCall);
    setupBoard({p1, p2, p3}, 1, 1300);

    board->distributePot();

    EXPECT_EQ(p1->getCash(), 650);
    EXPECT_EQ(p2->getCash(), 650);
    EXPECT_EQ(p3->getCash(), 0);
    EXPECT_EQ(board->getWinners().size(), 2);
}

TEST_F(BoardTest, FoldedPlayerGetsNothing)
{
    auto p1 = createPlayer(0, 500, 0, 100, PlayerActionCall);
    auto p2 = createPlayer(1, 500, 0, 90, PlayerActionCall);
    auto p3 = createPlayer(2, 500, 0, 120, PlayerActionFold);
    setupBoard({p1, p2, p3}, 0, 1500);

    board->distributePot();

    EXPECT_EQ(p3->getCash(), 0);
    EXPECT_NE(board->getWinners().front(), 2);
}

TEST_F(BoardTest, ThreeWaySplit)
{
    auto p1 = createPlayer(0, 500, 0, 100, PlayerActionCall);
    auto p2 = createPlayer(1, 500, 0, 100, PlayerActionCall);
    auto p3 = createPlayer(2, 500, 0, 100, PlayerActionCall);
    setupBoard({p1, p2, p3}, 1, 1500);

    board->distributePot();

    EXPECT_EQ(p1->getCash(), 500);
    EXPECT_EQ(p2->getCash(), 500);
    EXPECT_EQ(p3->getCash(), 500);
    EXPECT_EQ(board->getWinners().size(), 3);
}

TEST_F(BoardTest, DISABLED_OddPotExtraGoesToLeftOfDealer)
{
    auto p1 = createPlayer(0, 500, 0, 100, PlayerActionCall);
    auto p2 = createPlayer(1, 500, 0, 100, PlayerActionCall);
    setupBoard({p1, p2}, 0, 1001); // odd pot

    board->distributePot();

    // The player left of dealer is p1 (id 1), so he should get the extra chip
    EXPECT_TRUE(p1->getCash() == 500 || p1->getCash() == 501);
    EXPECT_TRUE(p2->getCash() == 500 || p2->getCash() == 501);
    EXPECT_EQ(p1->getCash() + p2->getCash(), 1001);
}

TEST_F(BoardTest, DISABLED_ShortStackedWinnerGetsOnlyMainPot)
{
    auto p1 = createPlayer(0, 200, 0, 100, PlayerActionCall); // Winner (short stack)
    auto p2 = createPlayer(1, 500, 0, 90, PlayerActionCall);
    auto p3 = createPlayer(2, 500, 0, 80, PlayerActionCall);

    // Pot = 200 (p1) + 200 (main pot from each player) + 300 + 300 (side pots)
    setupBoard({p1, p2, p3}, 1, 1200);

    board->distributePot();

    // p1 should win only the main pot: 200 * 3 = 600
    EXPECT_EQ(p1->getCash(), 600);
    // The rest (600) goes to the next best player (p2)
    EXPECT_GT(p2->getCash(), p3->getCash());
    EXPECT_EQ(board->getWinners().front(), 0); // p1 wins main pot
}
TEST_F(BoardTest, SidePotWinnerIsNotShortStackedWinner)
{
    auto p1 = createPlayer(0, 100, 0, 100, PlayerActionCall); // Tied winner (shortest stack)
    auto p2 = createPlayer(1, 500, 0, 100, PlayerActionCall); // Tied winner (larger stack)
    auto p3 = createPlayer(2, 500, 0, 70, PlayerActionCall);

    // All put at least 100 in, total pot = 1100
    setupBoard({p1, p2, p3}, 2, 1100);

    board->distributePot();

    // p1 can only win from main pot (300). Tied with p2.
    // p2 should win main pot with p1 (split), and side pot with p3.
    EXPECT_EQ(p1->getCash(), 150); // Half of main pot (300)
    EXPECT_EQ(p2->getCash(), 950); // 150 (main) + 800 (side)
    EXPECT_EQ(p3->getCash(), 0);

    // Check winners
    auto winners = board->getWinners();
    EXPECT_EQ(winners.size(), 2);
    EXPECT_TRUE(std::find(winners.begin(), winners.end(), 0) != winners.end());
    EXPECT_TRUE(std::find(winners.begin(), winners.end(), 1) != winners.end());
}
TEST_F(BoardTest, DISABLED_WinnerTakesOnlyEligiblePotPortion)
{
    auto p1 = createPlayer(0, 300, 0, 110, PlayerActionCall); // winner
    auto p2 = createPlayer(1, 500, 0, 90, PlayerActionCall);
    auto p3 = createPlayer(2, 700, 0, 70, PlayerActionCall);

    // All put in at least 300, so:
    // main pot = 300 x 3 = 900
    // side pot 1 = (500-300) + (700-300) = 600
    setupBoard({p1, p2, p3}, 0, 1500);

    board->distributePot();

    // p1 wins only the 900 main pot
    EXPECT_EQ(p1->getCash(), 900);

    // p2 and p3 split or resolve the 600 side pot, depending on cards
    EXPECT_EQ(board->getWinners().front(), 0);
}

} // namespace pkt::test
