#include "common/common.h"

#include "common/DummyPlayer.h"
#include "core/engine/PotFsm.h"
#include "core/engine/model/GameState.h"
#include "core/engine/round_states/RiverState.h"
#include "core/player/typedefs.h"

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

class PotTest : public ::testing::Test
{
  protected:
    PlayerFsmList seats;
    GameEvents events;
    std::unique_ptr<PotFsm> pot;
    std::unique_ptr<RiverState> state = std::make_unique<RiverState>(events);

    std::shared_ptr<DummyPlayerFsm> createPlayerFsm(int id, int cashAtHandStart, int remainingCash, int handRank,
                                                    ActionType action)
    {
        auto p = std::make_shared<DummyPlayerFsm>(id, events);
        p->setCashAtHandStart(cashAtHandStart);
        p->setCash(remainingCash);
        p->setHandRanking(handRank);
        p->setAction(*state, {id, action});

        return p;
    }

    void setupPot(std::initializer_list<std::shared_ptr<DummyPlayerFsm>> players, int potAmount, int dealerId)
    {
        seats = std::make_shared<std::list<std::shared_ptr<PlayerFsm>>>();
        for (const auto& p : players)
            seats->push_back(std::static_pointer_cast<PlayerFsm>(p));
        pot = std::make_unique<PotFsm>(potAmount, seats, dealerId);
    }
};

TEST_F(PotTest, SingleWinnerGetsFullPot)
{
    auto p1 = createPlayerFsm(0, 1000, 750, 200, ActionType::Call);
    auto p2 = createPlayerFsm(1, 1000, 750, 100, ActionType::Call);
    setupPot({p1, p2}, 500, 0);
    pot->distribute();

    EXPECT_EQ(p1->getCash(), 1250);
    EXPECT_EQ(p2->getCash(), 750);
}

TEST_F(PotTest, EqualSplitBetweenTwoWinners)
{
    auto p1 = createPlayerFsm(0, 1000, 750, 150, ActionType::Call);
    auto p2 = createPlayerFsm(1, 1000, 750, 150, ActionType::Call);
    setupPot({p1, p2}, 500, 0);
    pot->distribute();

    EXPECT_EQ(p1->getCash(), 1000);
    EXPECT_EQ(p2->getCash(), 1000);
}

TEST_F(PotTest, OddChipGoesToLeftOfDealer)
{
    auto p1 = createPlayerFsm(0, 1000, 0, 150, ActionType::Call);
    auto p2 = createPlayerFsm(1, 1000, 0, 150, ActionType::Call);
    setupPot({p1, p2}, 501, 0);
    pot->distribute();

    EXPECT_EQ(p1->getCash() + p2->getCash(), 501);
    EXPECT_TRUE(p1->getCash() == 250 || p1->getCash() == 251);
}

TEST_F(PotTest, FoldedPlayerGetsNothing2Players)
{
    auto p1 = createPlayerFsm(0, 1000, 300, 100, ActionType::Call);
    auto p2 = createPlayerFsm(1, 1000, 200, 120, ActionType::Fold);
    setupPot({p1, p2}, 500, 0);
    pot->distribute();

    EXPECT_EQ(p2->getCash(), 200);
}

TEST_F(PotTest, SidePotWithShortStack)
{
    auto p1 = createPlayerFsm(0, 100, 0, 150, ActionType::Call);
    auto p2 = createPlayerFsm(1, 300, 0, 100, ActionType::Call);
    auto p3 = createPlayerFsm(2, 300, 0, 120, ActionType::Call);
    setupPot({p1, p2, p3}, 700, 2);
    pot->distribute();

    EXPECT_EQ(p1->getCash(), 300);
    EXPECT_GT(p3->getCash(), p2->getCash());
}

TEST_F(PotTest, NoWinnersMeansNoDistribution)
{
    auto p1 = createPlayerFsm(0, 1000, 0, 100, ActionType::Fold);
    auto p2 = createPlayerFsm(1, 1000, 0, 100, ActionType::Fold);
    setupPot({p1, p2}, 500, 1);
    pot->distribute();

    EXPECT_EQ(p1->getCash(), 0);
    EXPECT_EQ(p2->getCash(), 0);
}

TEST_F(PotTest, AllInPlayerWinsSidePot)
{
    auto p1 = createPlayerFsm(0, 100, 0, 200, ActionType::Call); // All-in
    auto p2 = createPlayerFsm(1, 300, 0, 150, ActionType::Call);
    auto p3 = createPlayerFsm(2, 300, 0, 100, ActionType::Call);
    setupPot({p1, p2, p3}, 700, 2);
    pot->distribute();

    EXPECT_EQ(p1->getCash(), 300);           // Player 0 wins the side pot
    EXPECT_GT(p2->getCash(), p3->getCash()); // Player 1 wins more than Player 2
}
TEST_F(PotTest, FoldedPlayersGetsNothing3Players)
{
    auto p1 = createPlayerFsm(0, 1000, 700, 200, ActionType::Call);
    auto p2 = createPlayerFsm(1, 1000, 900, 150, ActionType::Fold);
    auto p3 = createPlayerFsm(2, 1000, 900, 100, ActionType::Call);
    setupPot({p1, p2, p3}, 500, 0);
    pot->distribute();

    EXPECT_EQ(p2->getCash(), 900);           // Folded player gets nothing
    EXPECT_GT(p1->getCash(), p3->getCash()); // Player 1 wins more than Player 3
}
TEST_F(PotTest, PotFullyDistributed)
{
    auto p1 = createPlayerFsm(0, 1000, 700, 200, ActionType::Call);
    auto p2 = createPlayerFsm(1, 1000, 900, 150, ActionType::Call);
    setupPot({p1, p2}, 400, 0);
    pot->distribute();

    EXPECT_EQ(p1->getCash() + p2->getCash(), 2000); // Total cash matches pot amount
}
TEST_F(PotTest, AllInPlayerWinsEntirePot)
{
    auto p1 = createPlayerFsm(0, 1000, 0, 200, ActionType::Call); // All-in
    auto p2 = createPlayerFsm(1, 1500, 500, 150, ActionType::Call);
    setupPot({p1, p2}, 1500, 0);
    pot->distribute();

    EXPECT_EQ(p1->getCash(), 1500); // Player 0 wins the entire pot
    EXPECT_EQ(p2->getCash(), 500);  // Player 1 gets nothing
}
} // namespace pkt::test
