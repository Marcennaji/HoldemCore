#include "common/EngineTest.h"
#include "core/engine/game/Board.h"
#include "core/player/Helpers.h"

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test {

class ShowdownRevealOrderTest : public EngineTest {
protected:
    void SetUp() override {
        EngineTest::SetUp();
        myEvents.clear();
    }
};

// Helper to set a player's cash directly to simulate total contribution (startCash - cash)
static void setContribution(std::shared_ptr<Player> p, int startCash, int desiredContribution) {
    const int targetCash = startCash - desiredContribution;
    p->setCash(targetCash);
}

TEST_F(ShowdownRevealOrderTest, NonAllIn_LastActorFirst_ThenByRankAndContribution)
{
    // Arrange: 3 players, we will set ranks and contributions to produce a clear order.
    initializeHandWithPlayers(3, gameData);

    auto& board = myHand->getBoard();
    // Seat/player IDs are 0,1,2 in creation order.
    auto p0 = getPlayerById(mySeatsList, 0);
    auto p1 = getPlayerById(mySeatsList, 1);
    auto p2 = getPlayerById(mySeatsList, 2);

    // Last action made by player 1
    board.setLastActionPlayerId(1);

    // Set explicit ranks (higher = better) and contributions
    // Start cash is 1000 by default in tests
    const int startCash = 1000;
    p1->setHandRanking(1000); setContribution(p1, startCash, 200); // last actor: (rank=1000, contrib=200)
    p2->setHandRanking(1500); setContribution(p2, startCash, 150); // higher rank
    p0->setHandRanking( 900); setContribution(p0, startCash, 250); // lower rank but higher contrib than level threshold

    // Act
    board.determineShowdownRevealOrder();
    const auto order = board.getShowdownRevealOrder();

    // Assert: last actor first, then p2 (higher rank), then p0 (lower rank but contrib>threshold)
    std::vector<unsigned> expected{1, 2, 0};
    EXPECT_EQ(order, expected);
}

TEST_F(ShowdownRevealOrderTest, AllIn_RevealsAllNonFoldersInSeatOrder)
{
    // Arrange: 3 players, all-in condition set; none folded (default last actions are None)
    initializeHandWithPlayers(3, gameData);
    auto& board = myHand->getBoard();
    board.setAllInCondition(true);

    // Act
    board.determineShowdownRevealOrder();
    const auto order = board.getShowdownRevealOrder();

    // Assert: everyone who didn't fold reveals in seat order (0,1,2)
    std::vector<unsigned> expected{0, 1, 2};
    EXPECT_EQ(order, expected);
}

TEST_F(ShowdownRevealOrderTest, LowerRankWithoutEnoughContribution_DoesNotReveal)
{
    // Arrange: 3 players, player 1 is last actor and sets the initial level threshold.
    initializeHandWithPlayers(3, gameData);
    auto& board = myHand->getBoard();
    auto p0 = getPlayerById(mySeatsList, 0);
    auto p1 = getPlayerById(mySeatsList, 1);
    auto p2 = getPlayerById(mySeatsList, 2);

    board.setLastActionPlayerId(1);

    const int startCash = 1000;
    p1->setHandRanking(1000); setContribution(p1, startCash, 200); // last actor: (1000, 200)
    p2->setHandRanking( 900); setContribution(p2, startCash, 150); // lower rank, insufficient contribution
    p0->setHandRanking(1100); setContribution(p0, startCash, 100); // higher rank later in order

    // Act
    board.determineShowdownRevealOrder();
    const auto order = board.getShowdownRevealOrder();

    // Assert: last actor (1), then player 0 (higher rank). Player 2 should not reveal.
    std::vector<unsigned> expected{1, 0};
    EXPECT_EQ(order, expected);
}

} // namespace pkt::test
