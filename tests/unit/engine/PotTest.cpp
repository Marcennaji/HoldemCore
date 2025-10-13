#include "common/common.h"

#include "common/DummyPlayer.h"
#include "core/engine/game/Pot.h"
#include "core/engine/model/GameState.h"
#include "core/engine/state/RiverState.h"
#include "adapters/infrastructure/statistics/NullPlayersStatisticsStore.h"
#include "core/player/typedefs.h"
#include "adapters/infrastructure/logger/NullLogger.h"
#include "adapters/infrastructure/randomizer/DefaultRandomizer.h"
#include "adapters/infrastructure/logger/ConsoleLogger.h"
#include "adapters/infrastructure/hand_evaluation/PsimHandEvaluationEngine.h"

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
    std::unique_ptr<RiverState> state;

    std::shared_ptr<pkt::core::Logger> m_logger;
    std::shared_ptr<pkt::core::HandEvaluationEngine> m_handEvaluationEngine;
    std::shared_ptr<pkt::core::PlayersStatisticsStore> m_playersStatisticsStore;
    std::shared_ptr<pkt::core::Randomizer> m_randomizer;

    void SetUp() override
    {
        auto logger = std::make_shared<pkt::infra::ConsoleLogger>();
        logger->setLogLevel(pkt::core::LogLevel::Info);
        m_logger = logger;
        m_handEvaluationEngine = std::make_shared<pkt::infra::PsimHandEvaluationEngine>();
        m_playersStatisticsStore = std::make_shared<pkt::infra::NullPlayersStatisticsStore>();
        m_randomizer = std::make_shared<pkt::core::DefaultRandomizer>();
        state = std::make_unique<RiverState>(events, *m_logger);
    }

    std::shared_ptr<DummyPlayer> createPlayer(int id, int cashAtHandStart, int remainingCash, int handRank,
                                              ActionType action)
    {
        auto p = std::make_shared<DummyPlayer>(id, events, m_logger, m_handEvaluationEngine, m_playersStatisticsStore, m_randomizer);
        p->setCashAtHandStart(cashAtHandStart);
        p->setCash(remainingCash);
        p->setHandRanking(handRank);
        p->setAction(*state, {id, action});

        return p;
    }

    void setupPot(std::initializer_list<std::shared_ptr<DummyPlayer>> players, int potAmount, int dealerId)
    {
        seats = std::make_shared<std::list<std::shared_ptr<Player>>>();
        for (const auto& p : players)
            seats->push_back(std::static_pointer_cast<Player>(p));
        
        // Reuse m_logger from SetUp() to avoid creating unnecessary logger instances
        pot = std::make_unique<Pot>(potAmount, seats, dealerId, *m_logger);
    }
};

TEST_F(PotTest, SingleWinnerGetsFullPot)
{
    auto p1 = createPlayer(0, 1000, 750, 200, ActionType::Call);
    auto p2 = createPlayer(1, 1000, 750, 100, ActionType::Call);
    setupPot({p1, p2}, 500, 0);
    pot->distribute();

    EXPECT_EQ(p1->getCash(), 1250);
    EXPECT_EQ(p2->getCash(), 750);
}

TEST_F(PotTest, EqualSplitBetweenTwoWinners)
{
    auto p1 = createPlayer(0, 1000, 750, 150, ActionType::Call);
    auto p2 = createPlayer(1, 1000, 750, 150, ActionType::Call);
    setupPot({p1, p2}, 500, 0);
    pot->distribute();

    EXPECT_EQ(p1->getCash(), 1000);
    EXPECT_EQ(p2->getCash(), 1000);
}

TEST_F(PotTest, OddChipGoesToLeftOfDealer)
{
    auto p1 = createPlayer(0, 1000, 0, 150, ActionType::Call);
    auto p2 = createPlayer(1, 1000, 0, 150, ActionType::Call);
    setupPot({p1, p2}, 501, 0);
    pot->distribute();

    EXPECT_EQ(p1->getCash() + p2->getCash(), 501);
    EXPECT_TRUE(p1->getCash() == 250 || p1->getCash() == 251);
}

TEST_F(PotTest, FoldedPlayerGetsNothing2Players)
{
    auto p1 = createPlayer(0, 1000, 300, 100, ActionType::Call);
    auto p2 = createPlayer(1, 1000, 200, 120, ActionType::Fold);
    setupPot({p1, p2}, 500, 0);
    pot->distribute();

    EXPECT_EQ(p2->getCash(), 200);
}

TEST_F(PotTest, SidePotWithShortStack)
{
    auto p1 = createPlayer(0, 100, 0, 150, ActionType::Call);
    auto p2 = createPlayer(1, 300, 0, 100, ActionType::Call);
    auto p3 = createPlayer(2, 300, 0, 120, ActionType::Call);
    setupPot({p1, p2, p3}, 700, 2);
    pot->distribute();

    EXPECT_EQ(p1->getCash(), 300);
    EXPECT_GT(p3->getCash(), p2->getCash());
}

TEST_F(PotTest, NoWinnersMeansNoDistribution)
{
    auto p1 = createPlayer(0, 1000, 0, 100, ActionType::Fold);
    auto p2 = createPlayer(1, 1000, 0, 100, ActionType::Fold);
    setupPot({p1, p2}, 500, 1);
    pot->distribute();

    EXPECT_EQ(p1->getCash(), 0);
    EXPECT_EQ(p2->getCash(), 0);
}

TEST_F(PotTest, AllInPlayerWinsSidePot)
{
    auto p1 = createPlayer(0, 100, 0, 200, ActionType::Call); // All-in
    auto p2 = createPlayer(1, 300, 0, 150, ActionType::Call);
    auto p3 = createPlayer(2, 300, 0, 100, ActionType::Call);
    setupPot({p1, p2, p3}, 700, 2);
    pot->distribute();

    EXPECT_EQ(p1->getCash(), 300);           // Player 0 wins the side pot
    EXPECT_GT(p2->getCash(), p3->getCash()); // Player 1 wins more than Player 2
}
TEST_F(PotTest, FoldedPlayersGetsNothing3Players)
{
    auto p1 = createPlayer(0, 1000, 700, 200, ActionType::Call);
    auto p2 = createPlayer(1, 1000, 900, 150, ActionType::Fold);
    auto p3 = createPlayer(2, 1000, 900, 100, ActionType::Call);
    setupPot({p1, p2, p3}, 500, 0);
    pot->distribute();

    EXPECT_EQ(p2->getCash(), 900);           // Folded player gets nothing
    EXPECT_GT(p1->getCash(), p3->getCash()); // Player 1 wins more than Player 3
}
TEST_F(PotTest, PotFullyDistributed)
{
    auto p1 = createPlayer(0, 1000, 700, 200, ActionType::Call);
    auto p2 = createPlayer(1, 1000, 900, 150, ActionType::Call);
    setupPot({p1, p2}, 400, 0);
    pot->distribute();

    EXPECT_EQ(p1->getCash() + p2->getCash(), 2000); // Total cash matches pot amount
}
TEST_F(PotTest, AllInPlayerWinsEntirePot)
{
    auto p1 = createPlayer(0, 1000, 0, 200, ActionType::Call); // All-in
    auto p2 = createPlayer(1, 1500, 500, 150, ActionType::Call);
    setupPot({p1, p2}, 1500, 0);
    pot->distribute();

    EXPECT_EQ(p1->getCash(), 1500); // Player 0 wins the entire pot
    EXPECT_EQ(p2->getCash(), 500);  // Player 1 gets nothing
}

// Tests for GUI winner display (getWinners() should only return absolute best hand)
TEST_F(PotTest, OnlyBestHandShownAsWinner_NotAllSidePotWinners)
{
    // Create 3 players with different chip amounts and hand rankings
    // Player 0: 1000 chips, rank 100 (weakest hand) - wins only final side pot
    // Player 1: 500 chips, rank 200 (medium hand) - all-in, wins nothing (loses to p2)
    // Player 2: 800 chips, rank 300 (strongest hand) - wins main pot + side pot 1
    
    auto p0 = createPlayer(0, 1000, 0, 100, ActionType::Call);  // Weakest
    auto p1 = createPlayer(1, 500, 0, 200, ActionType::Call);   // Medium
    auto p2 = createPlayer(2, 800, 0, 300, ActionType::Call);   // Strongest
    
    setupPot({p0, p1, p2}, 2300, 0);
    pot->distribute();
    
    // Only player 2 (strongest hand) should be shown as winner in GUI
    const auto& winners = pot->getWinners();
    
    ASSERT_EQ(winners.size(), 1) << "Only one player should be shown as winner in GUI";
    EXPECT_EQ(winners.front(), 2) << "Player 2 with strongest hand should be the winner";
    
    // Verify chip distribution:
    // Main pot (500x3=1500): Player 2 wins
    // Side pot 1 (300x2=600): Player 2 wins (p0 and p2 eligible, p2 has better hand)
    // Side pot 2 (200x1=200): Player 0 wins (only p0 eligible)
    EXPECT_EQ(p2->getCash(), 2100) << "Player 2 should win main pot + side pot 1";
    EXPECT_EQ(p0->getCash(), 200) << "Player 0 should win only final side pot";
    EXPECT_EQ(p1->getCash(), 0) << "Player 1 should win nothing";
}

TEST_F(PotTest, MultipleWinnersShown_WhenHandsAreTied)
{
    // Create 3 players where 2 have tied best hands
    auto p0 = createPlayer(0, 1000, 0, 500, ActionType::Call);  // Tied best
    auto p1 = createPlayer(1, 1000, 0, 500, ActionType::Call);  // Tied best
    auto p2 = createPlayer(2, 1000, 0, 100, ActionType::Call);  // Weaker
    
    setupPot({p0, p1, p2}, 3000, 0);
    pot->distribute();
    
    // Both players 0 and 1 should be shown as winners
    const auto& winners = pot->getWinners();
    
    EXPECT_EQ(winners.size(), 2) << "Two players with tied hands should both be shown as winners";
    
    std::vector<unsigned> winnerVec(winners.begin(), winners.end());
    EXPECT_TRUE(std::find(winnerVec.begin(), winnerVec.end(), 0) != winnerVec.end()) 
        << "Player 0 should be in winners";
    EXPECT_TRUE(std::find(winnerVec.begin(), winnerVec.end(), 1) != winnerVec.end()) 
        << "Player 1 should be in winners";
    EXPECT_TRUE(std::find(winnerVec.begin(), winnerVec.end(), 2) == winnerVec.end()) 
        << "Player 2 should NOT be in winners";
}

TEST_F(PotTest, FoldedPlayersNeverShownAsWinners)
{
    // Player with best hand folded, weaker hand wins
    auto p0 = createPlayer(0, 1000, 900, 500, ActionType::Fold);  // Stronger but folded
    auto p1 = createPlayer(1, 1000, 0, 200, ActionType::Call);    // Weaker but didn't fold
    
    setupPot({p0, p1}, 1100, 0);
    pot->distribute();
    
    // Only player 1 should be shown as winner (player 0 folded)
    const auto& winners = pot->getWinners();
    
    ASSERT_EQ(winners.size(), 1) << "Only one player should be shown as winner";
    EXPECT_EQ(winners.front(), 1) << "Player 1 should be the winner (player 0 folded)";
    
    // Player 1 should get all the pot
    EXPECT_GT(p1->getCash(), 1000) << "Player 1 should have won chips";
    EXPECT_EQ(p0->getCash(), 900) << "Player 0 (folded) should not win anything";
}

TEST_F(PotTest, SidePotWinnerNotShownUnlessHasBestHand)
{
    // Player 0: Short stack, has weakest hand
    // Player 1: Medium hand
    // Player 2: Best hand, wins everything
    
    auto p0 = createPlayer(0, 100, 0, 100, ActionType::Call);   // Weakest, short stack
    auto p1 = createPlayer(1, 300, 0, 200, ActionType::Call);   // Medium
    auto p2 = createPlayer(2, 300, 0, 300, ActionType::Call);   // Strongest
    
    setupPot({p0, p1, p2}, 700, 2);
    pot->distribute();
    
    // Only player 2 (strongest hand) should be shown as winner
    const auto& winners = pot->getWinners();
    
    ASSERT_EQ(winners.size(), 1) << "Only player with absolute best hand should be shown";
    EXPECT_EQ(winners.front(), 2) << "Player 2 with best hand should be shown as winner";
    
    // Pot distribution:
    // Main pot (100x3=300): Player 2 wins (best hand)
    // Side pot (200x2=400): Player 2 wins (p1 and p2 eligible, p2 has best hand)
    EXPECT_EQ(p2->getCash(), 700) << "Player 2 should win entire pot";
    EXPECT_EQ(p1->getCash(), 0) << "Player 1 should win nothing";
    EXPECT_EQ(p0->getCash(), 0) << "Player 0 should win nothing";
}

} // namespace pkt::test
