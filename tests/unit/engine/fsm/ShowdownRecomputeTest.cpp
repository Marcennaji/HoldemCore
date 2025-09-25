#include "common/EngineTest.h"
#include "core/engine/cards/Card.h"
#include "core/player/Helpers.h"

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test {

class ShowdownRecomputeTest : public EngineTest {
protected:
    void SetUp() override {
        EngineTest::SetUp();
        myEvents.clear();
        myEvents.onBettingRoundStarted = [&](GameState state) { myLastGameState = state; };
    }
};

// This test ensures that when players have valid hole cards and a full board is present,
// ranks are recomputed at showdown using HOLE + BOARD, yielding a correct single winner.
TEST_F(ShowdownRecomputeTest, RecomputeRanksWithValidHoleCards_RealShowdown)
{
    // Arrange: 2 players, explicit valid hole cards and full board
    initializeHandWithPlayers(2, gameData);

    auto p0 = getPlayerById(myActingPlayersList, 0); // SB
    auto p1 = getPlayerById(myActingPlayersList, 1); // BB

    // Set a clean board with no straights/flushes possible to keep evaluation simple
    // Board: 2c 7d 9h 4s 3c
    BoardCards boardCards("2c", "7d", "9h", "4s", "3c");
    myHand->getBoard().setBoardCards(boardCards);

    // Give Player 0 pocket Aces (clearly stronger than Kings on this board)
    p0->setHoleCards(Card("Ah"), Card("Ad"));
    // Give Player 1 pocket Kings
    p1->setHoleCards(Card("Kh"), Card("Kd"));

    // Pre-set identical ranks to simulate stale/pre-showdown values.
    // At showdown, recomputation must override these with distinct values.
    p0->setHandRanking(1000);
    p1->setHandRanking(1000);

    // Act: Check through to showdown
    myHand->handlePlayerAction({p0->getId(), ActionType::Call});
    myHand->handlePlayerAction({p1->getId(), ActionType::Check});
    myHand->handlePlayerAction({p0->getId(), ActionType::Check});
    myHand->handlePlayerAction({p1->getId(), ActionType::Check});
    myHand->handlePlayerAction({p0->getId(), ActionType::Check});
    myHand->handlePlayerAction({p1->getId(), ActionType::Check});
    myHand->handlePlayerAction({p0->getId(), ActionType::Check});
    myHand->handlePlayerAction({p1->getId(), ActionType::Check});

    // Assert: We reached PostRiver and recomputation occurred
    EXPECT_EQ(myLastGameState, PostRiver);

    // Verify one winner and it's player 0 (Aces over Kings)
    const auto winners = myHand->getBoard().getWinners();
    ASSERT_EQ(winners.size(), 1u);
    EXPECT_EQ(winners.front(), p0->getId());

    // Verify ranks are no longer equal and winner's rank is higher
    const int r0 = p0->getHandRanking();
    const int r1 = p1->getHandRanking();
    EXPECT_NE(r0, r1);
    EXPECT_GT(r0, r1);
}

} // namespace pkt::test
