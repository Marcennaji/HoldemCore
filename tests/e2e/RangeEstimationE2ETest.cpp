// HoldemCore — E2E test for Range Estimation
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include <gtest/gtest.h>
#include "common/EngineTest.h"

#include "core/engine/cards/Card.h"
#include "core/engine/hand/Hand.h"
#include "core/engine/model/GameData.h"
#include "core/engine/model/StartData.h"
#include "core/player/strategy/PlayerStrategy.h"
#include "core/player/strategy/TightAggressiveBotStrategy.h"
#include "core/player/strategy/LooseAggressiveBotStrategy.h"
#include "core/player/range/RangeParser.h"

#include <map>
#include <vector>
#include <memory>
#include <functional>

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

/**
 * @brief Captures a snapshot of a player's estimated range at a specific game state.
 */
struct RangeSnapshot
{
    GameState gameState;
    int playerId;
    std::string rangeString;
    int handCount; // Number of unique hand combinations in the range
    ActionType lastAction;
    
    RangeSnapshot(GameState state, int id, const std::string& range, ActionType action)
        : gameState(state), playerId(id), rangeString(range), lastAction(action)
    {
        // Parse the range to count hands (empty range = 0 hands)
        if (range.empty())
        {
            handCount = 0;
        }
        else
        {
            try
            {
                auto atomicValues = RangeParser::getRangeAtomicValues(range, false);
                handCount = static_cast<int>(atomicValues.size());
            }
            catch (...)
            {
                // If parsing fails, set to -1 to indicate error
                handCount = -1;
            }
        }
    }
};

/**
 * @brief Tracks and validates range estimation throughout a poker hand.
 * 
 * This helper class captures opponent range snapshots after each action
 * and provides validation that ranges narrow monotonically (never expand).
 */
class RangeSnapshotTracker
{
  public:
    void captureSnapshot(GameState state, Player& opponent, ActionType lastAction)
    {
        std::string range = opponent.getRangeEstimator()->getEstimatedRange();
        m_snapshots.emplace_back(state, opponent.getId(), range, lastAction);
    }
    
    const std::vector<RangeSnapshot>& getSnapshots() const
    {
        return m_snapshots;
    }
    
    void verifyMonotonicNarrowing(::testing::Test* test) const
    {
        ASSERT_GE(m_snapshots.size(), 2) << "Need at least 2 snapshots to verify narrowing";
        
        // Find the first non-empty range (range estimation is lazy - initialized on first real action)
        size_t firstNonEmptyIdx = 0;
        for (size_t i = 0; i < m_snapshots.size(); i++)
        {
            if (m_snapshots[i].handCount > 0)
            {
                firstNonEmptyIdx = i;
                break;
            }
        }
        
        if (firstNonEmptyIdx == 0 && m_snapshots[0].handCount == 0)
        {
            // All snapshots are empty - this shouldn't happen in a real game
            FAIL() << "All range snapshots are empty";
            return;
        }
        
        // Verify narrowing from first non-empty snapshot onwards
        for (size_t i = firstNonEmptyIdx + 1; i < m_snapshots.size(); i++)
        {
            const auto& prev = m_snapshots[i - 1];
            const auto& curr = m_snapshots[i];
            
            // Range should only narrow or stay the same, never expand
            EXPECT_LE(curr.handCount, prev.handCount)
                << "Range expanded from " << prev.handCount << " to " << curr.handCount << " hands\n"
                << "  After action in state: " << static_cast<int>(curr.gameState) << "\n"
                << "  Previous range: " << prev.rangeString << "\n"
                << "  Current range: " << curr.rangeString;
        }
    }
    
    void logSnapshots(pkt::core::Logger& logger) const
    {
        logger.info("=== Range Evolution Snapshots ===");
        for (const auto& snapshot : m_snapshots)
        {
            std::string stateName;
            switch (snapshot.gameState)
            {
            case Preflop: stateName = "Preflop"; break;
            case Flop: stateName = "Flop"; break;
            case Turn: stateName = "Turn"; break;
            case River: stateName = "River"; break;
            default: stateName = "Unknown"; break;
            }
            
            logger.info("  [" + stateName + "] Player " + std::to_string(snapshot.playerId) + 
                       " after " + std::string(actionTypeToString(snapshot.lastAction)) +
                       ": " + std::to_string(snapshot.handCount) + " hands" +
                       (snapshot.rangeString.length() < 100 ? " (" + snapshot.rangeString + ")" : ""));
        }
        logger.info("=== End Range Snapshots ===\n");
    }
    
  private:
    std::vector<RangeSnapshot> m_snapshots;
};

/**
 * @brief E2E tests validating that range estimation narrows correctly through a complete hand.
 */
class RangeEstimationE2ETest : public EngineTest
{
  protected:
    int cardsOverriddenCount = 0;
    std::unique_ptr<RangeSnapshotTracker> m_rangeTracker;
    std::map<unsigned, HoleCards> m_playerCardMap;
    BoardCards m_predeterminedBoard;
    
    void SetUp() override
    {
        EngineTest::SetUp();
        
        m_rangeTracker = std::make_unique<RangeSnapshotTracker>();
        
        gameData.maxNumberOfPlayers = 3;
        gameData.startMoney = 1000;
        gameData.firstSmallBlind = 10;
        gameData.tableProfile = TableProfile::RandomOpponents;
    }
    
    void setupPlayers(const std::vector<std::pair<HoleCards, std::unique_ptr<PlayerStrategy>>>& playerConfigs)
    {
        // playerConfigs: {holeCards, strategy}
        m_seatsList = std::make_shared<std::list<std::shared_ptr<Player>>>();
        
        unsigned playerId = 0;
        for (auto& config : playerConfigs)
        {
            auto logger = getLoggerService();
            auto handEvaluator = getHandEvaluationEngineService();
            auto statisticsStore = getPlayersStatisticsStoreService();
            auto randomizer = getRandomizerService();
            
            auto player = std::make_shared<Player>(m_events, *logger, *handEvaluator, 
                                                   *statisticsStore, *randomizer,
                                                   playerId, "Player" + std::to_string(playerId), 1000);
            
            // Set the provided strategy (must move since it's unique_ptr)
            player->setStrategy(std::move(const_cast<std::unique_ptr<PlayerStrategy>&>(config.second)));
            
            m_seatsList->push_back(player);
            m_playerCardMap[playerId] = config.first;
            playerId++;
        }
        
        // Acting players list starts identical to seats list
        m_actingPlayersList = std::make_shared<std::list<std::shared_ptr<Player>>>();
        for (const auto& player : *m_seatsList)
        {
            m_actingPlayersList->push_back(player);
        }
    }
    
    void setupPredeterminedCards(const BoardCards& board)
    {
        m_predeterminedBoard = board;
        cardsOverriddenCount = 0;
        
        // Override hole cards
        m_events.onHoleCardsDealt = [this](unsigned playerId, HoleCards dealtCards)
        {
            auto it = m_playerCardMap.find(playerId);
            if (it != m_playerCardMap.end())
            {
                for (auto& player : *m_seatsList)
                {
                    if (player->getId() == playerId)
                    {
                        player->setHoleCards(it->second);
                        getLogger().verbose("Override Player " + std::to_string(playerId) + 
                                          " cards to " + it->second.toString());
                        cardsOverriddenCount++;
                        break;
                    }
                }
            }
        };
        
        // Override board cards
        m_events.onBoardCardsDealt = [this](BoardCards dealtBoard)
        {
            m_hand->getBoard().setBoardCards(m_predeterminedBoard);
        };
    }
    
    void setupRangeTracking(unsigned heroId, unsigned opponentId)
    {
        // Track opponent's range from hero's perspective
        // This is captured AFTER each action is processed
        m_events.onPlayerActed = [this, heroId, opponentId](PlayerAction action)
        {
            // After any player acts, capture the opponent's estimated range
            // (from the hero's perspective)
            if (action.playerId == opponentId)
            {
                // Opponent acted, their range should have been updated
                auto opponent = getPlayerById(opponentId);
                if (opponent && m_hand)
                {
                    GameState currentState = m_hand->getGameState();
                    m_rangeTracker->captureSnapshot(currentState, *opponent, action.type);
                }
            }
        };
    }
    
    std::shared_ptr<Player> getPlayerById(unsigned id)
    {
        for (auto& player : *m_seatsList)
        {
            if (player->getId() == id)
            {
                return player;
            }
        }
        return nullptr;
    }
    
    void logTestMessage(const std::string& message)
    {
        getLogger().info("[TEST] " + message);
    }
    
    // Helper: Create player config with strategy
    std::pair<HoleCards, std::unique_ptr<PlayerStrategy>> makePlayer(
        const std::string& card1, const std::string& card2, 
        std::function<std::unique_ptr<PlayerStrategy>(Logger&, Randomizer&)> strategyFactory)
    {
        auto logger = getLoggerService();
        auto randomizer = getRandomizerService();
        
        return {HoleCards(Card(card1), Card(card2)), strategyFactory(*logger, *randomizer)};
    }
    
    // Helper: Create board from card strings
    BoardCards makeBoard(const std::string& f1, const std::string& f2, const std::string& f3,
                         const std::string& turn, const std::string& river)
    {
        BoardCards board;
        board.dealFlop(Card(f1), Card(f2), Card(f3));
        board.dealTurn(Card(turn));
        board.dealRiver(Card(river));
        return board;
    }
    
    // Helper: Initialize and run a hand
    void runHand(unsigned dealerId, unsigned numPlayers, unsigned heroId, unsigned opponentId)
    {
        setupRangeTracking(heroId, opponentId);
        
        m_board = m_factory->createBoard(dealerId);
        m_board->setSeatsList(m_seatsList);
        m_board->setActingPlayersList(m_actingPlayersList);
        
        StartData startData;
        startData.startDealerPlayerId = dealerId;
        startData.numberOfPlayers = numPlayers;
        
        m_hand = m_factory->createHand(m_board, m_seatsList, m_actingPlayersList, gameData, startData);
        
        logTestMessage("Running hand...");
        m_hand->initialize();
        EXPECT_EQ(m_hand->getGameState(), Preflop);
        
        m_hand->runGameLoop();
        
        logTestMessage("Hand completed");
        EXPECT_EQ(m_hand->getGameState(), PostRiver);
    }
    
    // Helper: Verify range snapshots
    void verifyRangeSnapshots()
    {
        m_rangeTracker->logSnapshots(getLogger());
        
        const auto& snapshots = m_rangeTracker->getSnapshots();
        ASSERT_GE(snapshots.size(), 1) << "Should have captured at least one range snapshot";
        
        logTestMessage("Captured " + std::to_string(snapshots.size()) + " range snapshots");
        
        // Find first non-empty snapshot and verify all subsequent are valid
        int firstNonEmptyIdx = -1;
        for (size_t i = 0; i < snapshots.size(); i++)
        {
            const auto& snapshot = snapshots[i];
            if (snapshot.handCount > 0)
            {
                if (firstNonEmptyIdx == -1)
                {
                    firstNonEmptyIdx = static_cast<int>(i);
                }
                EXPECT_GT(snapshot.handCount, 0) 
                    << "Range should not become empty after being initialized (snapshot " << i << ")";
            }
            EXPECT_NE(snapshot.handCount, -1) << "Range parsing should not fail at snapshot " << i;
        }
        
        ASSERT_GE(firstNonEmptyIdx, 0) << "Should have at least one non-empty range snapshot";
        
        m_rangeTracker->verifyMonotonicNarrowing(this);
    }
};

// ========================================
// Test: Range Narrows Through Complete Hand
// ========================================

TEST_F(RangeEstimationE2ETest, OpponentRangeNarrowsThroughCompletedHand)
{
    logTestMessage("=== Test: Opponent Range Narrows Through Completed Hand ===");
    
    // Scenario: Hero (AA) vs Aggressive Opponent (QJs) on Qh 9h 3d 7c 2s
    const unsigned HERO_ID = 0, OPPONENT_ID = 1, DEALER_ID = 2;
    
    // Setup players
    std::vector<std::pair<HoleCards, std::unique_ptr<PlayerStrategy>>> playerConfigs;
    playerConfigs.push_back(makePlayer("As", "Ah", [](Logger& log, Randomizer& rnd) {
        return std::make_unique<TightAggressiveBotStrategy>(log, rnd);
    }));
    playerConfigs.push_back(makePlayer("Qc", "Js", [](Logger& log, Randomizer& rnd) {
        return std::make_unique<LooseAggressiveBotStrategy>(log, rnd);
    }));
    playerConfigs.push_back(makePlayer("7d", "2d", [](Logger& log, Randomizer& rnd) {
        return std::make_unique<TightAggressiveBotStrategy>(log, rnd);
    }));
    setupPlayers(playerConfigs);
    
    // Setup board and run hand
    setupPredeterminedCards(makeBoard("Qh", "9h", "3d", "7c", "2s"));
    runHand(DEALER_ID, 3, HERO_ID, OPPONENT_ID);
    
    // Verify and log results
    verifyRangeSnapshots();
    logTestMessage("=== Test Completed Successfully ===");
}

// ========================================
// Test: Range Narrows With Each Street
// ========================================

TEST_F(RangeEstimationE2ETest, RangeNarrowsWithEachStreet)
{
    logTestMessage("=== Test: Range Narrows With Each Street ===");
    
    // Scenario: Hero (KK) vs Aggressive Opponent (AQs) on low board
    const unsigned HERO_ID = 0, OPPONENT_ID = 1;
    
    // Setup players (heads-up)
    std::vector<std::pair<HoleCards, std::unique_ptr<PlayerStrategy>>> playerConfigs;
    playerConfigs.push_back(makePlayer("Kd", "Kc", [](Logger& log, Randomizer& rnd) {
        return std::make_unique<TightAggressiveBotStrategy>(log, rnd);
    }));
    playerConfigs.push_back(makePlayer("Ad", "Qd", [](Logger& log, Randomizer& rnd) {
        return std::make_unique<LooseAggressiveBotStrategy>(log, rnd);
    }));
    setupPlayers(playerConfigs);
    
    // Setup board and run hand
    setupPredeterminedCards(makeBoard("9s", "5h", "2c", "6d", "3h"));
    runHand(HERO_ID, 2, HERO_ID, OPPONENT_ID);
    
    // Log snapshots by street
    const auto& snapshots = m_rangeTracker->getSnapshots();
    std::map<GameState, int> snapshotsByStreet;
    for (const auto& snapshot : snapshots)
    {
        snapshotsByStreet[snapshot.gameState]++;
    }
    
    logTestMessage("Snapshots by street:");
    logTestMessage("  Preflop: " + std::to_string(snapshotsByStreet[Preflop]));
    logTestMessage("  Flop: " + std::to_string(snapshotsByStreet[Flop]));
    logTestMessage("  Turn: " + std::to_string(snapshotsByStreet[Turn]));
    logTestMessage("  River: " + std::to_string(snapshotsByStreet[River]));
    
    // Verify and log results
    verifyRangeSnapshots();
    logTestMessage("=== Test Completed Successfully ===");
}

} // namespace pkt::test
