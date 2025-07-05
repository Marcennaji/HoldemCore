// tests/engine/PotTest.cpp

#include "core/engine/Pot.h"
#include <gtest/gtest.h>
#include "DummyPlayer.h"
#include "core/player/typedefs.h"

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

class PotTest : public ::testing::Test
{
  protected:
    PlayerList seats;
    GameEvents events;
    std::unique_ptr<Pot> pot;

    std::shared_ptr<DummyPlayer> createPlayer(int id, int startCash, int cash, int handRank, PlayerAction action)
    {
        auto p = std::make_shared<DummyPlayer>(id, events);
        p->setId(id);
        p->setRoundStartCash(startCash);
        p->setCash(cash);
        p->setHandRanking(handRank);
        p->setAction(action);
        return p;
    }

    void setupPot(std::initializer_list<std::shared_ptr<DummyPlayer>> players, int potAmount, int dealerId)
    {
        seats = std::make_shared<std::list<std::shared_ptr<Player>>>();
        for (const auto& p : players)
            seats->push_back(std::static_pointer_cast<Player>(p));
        pot = std::make_unique<Pot>(potAmount, seats, dealerId);
    }
};

TEST_F(PotTest, SingleWinnerGetsFullPot)
{
    auto p1 = createPlayer(0, 1000, 0, 200, PlayerActionCall);
    auto p2 = createPlayer(1, 1000, 0, 100, PlayerActionCall);
    setupPot({p1, p2}, 500, 0);
    pot->distribute();

    EXPECT_EQ(p1->getCash(), 500);
    EXPECT_EQ(p2->getCash(), 0);
}

TEST_F(PotTest, EqualSplitBetweenTwoWinners)
{
    auto p1 = createPlayer(0, 1000, 0, 150, PlayerActionCall);
    auto p2 = createPlayer(1, 1000, 0, 150, PlayerActionCall);
    setupPot({p1, p2}, 500, 0);
    pot->distribute();

    EXPECT_EQ(p1->getCash(), 250);
    EXPECT_EQ(p2->getCash(), 250);
}

TEST_F(PotTest, OddChipGoesToLeftOfDealer)
{
    auto p1 = createPlayer(0, 1000, 0, 150, PlayerActionCall);
    auto p2 = createPlayer(1, 1000, 0, 150, PlayerActionCall);
    setupPot({p1, p2}, 501, 0);
    pot->distribute();

    EXPECT_EQ(p1->getCash() + p2->getCash(), 501);
    EXPECT_TRUE(p1->getCash() == 250 || p1->getCash() == 251);
}

TEST_F(PotTest, FoldedPlayerGetsNothing)
{
    auto p1 = createPlayer(0, 1000, 0, 100, PlayerActionCall);
    auto p2 = createPlayer(1, 1000, 0, 120, PlayerActionFold);
    setupPot({p1, p2}, 500, 0);
    pot->distribute();

    EXPECT_EQ(p2->getCash(), 0);
}

TEST_F(PotTest, SidePotWithShortStack)
{
    auto p1 = createPlayer(0, 100, 0, 150, PlayerActionCall);
    auto p2 = createPlayer(1, 300, 0, 100, PlayerActionCall);
    auto p3 = createPlayer(2, 300, 0, 120, PlayerActionCall);
    setupPot({p1, p2, p3}, 700, 2);
    pot->distribute();

    EXPECT_EQ(p1->getCash(), 300);
    EXPECT_GT(p3->getCash(), p2->getCash());
}

TEST_F(PotTest, NoWinnersMeansNoDistribution)
{
    auto p1 = createPlayer(0, 1000, 0, 100, PlayerActionFold);
    auto p2 = createPlayer(1, 1000, 0, 100, PlayerActionFold);
    setupPot({p1, p2}, 500, 1);
    pot->distribute();

    EXPECT_EQ(p1->getCash(), 0);
    EXPECT_EQ(p2->getCash(), 0);
}

} // namespace pkt::test
