#include <gtest/gtest.h>

#include "common/EngineTest.h"
#include "common/DummyPlayer.h"
#include "core/engine/utils/Helpers.h"
#include "core/engine/model/PlayerPosition.h"
#include "infra/persistence/NullPlayersStatisticsStore.h"
#include "core/player/Helpers.h"
#include "core/player/Player.h"
#include "core/services/DefaultRandomizer.h"
#include "infra/ConsoleLogger.h"
#include "infra/eval/PsimHandEvaluationEngine.h"

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test {

// Sanity check for circular offset wrap-around computation
TEST(PositioningUnit, CircularOffsetWrapAround)
{
    auto logger = std::make_shared<pkt::infra::ConsoleLogger>();
    logger->setLogLevel(pkt::core::LogLevel::Info);
    auto handEvaluationEngine = std::make_shared<pkt::infra::PsimHandEvaluationEngine>();
    auto statisticsStore = std::make_shared<pkt::infra::NullPlayersStatisticsStore>();
    auto randomizer = std::make_shared<pkt::core::DefaultRandomizer>();

    // Build a simple list of 6 dummy players with ids 0..5
    auto players = std::make_shared<std::list<std::shared_ptr<Player>>>();
    GameEvents ev; // unused
    for (int i = 0; i < 6; ++i) {
        players->push_back(std::make_shared<DummyPlayer>(i, ev, logger, handEvaluationEngine, statisticsStore, randomizer));
    }

    // Dealer = 4, To = 0; distance should wrap: indices [4 -> 5 -> 0] => 2
    int dist = playerDistanceCircularOffset(4, 0, players);
    EXPECT_EQ(dist, 2) << "Expected wrap-around distance 2 from dealer 4 to player 0";
}

// Verify position mapping by offset for a 6-max table
TEST(PositioningUnit, PositionMappingSixMax)
{
    const int playerCount = 6;
    // Expected positions for 6-max by offset from dealer:
    // 0: Button, 1: SB, 2: BB, 3: UTG, 4: Middle, 5: Cutoff
    std::vector<PlayerPosition> expected = {
        PlayerPosition::Button,
        PlayerPosition::SmallBlind,
        PlayerPosition::BigBlind,
        PlayerPosition::UnderTheGun,
        PlayerPosition::Middle,
        PlayerPosition::Cutoff,
    };

    for (int offset = 0; offset < playerCount; ++offset) {
        auto pos = computePlayerPositionFromOffset(offset, playerCount);
        EXPECT_EQ(pos, expected[offset]) << "Mismatch at offset " << offset;
    }
}

// Integration-ish test: ensure first to act preflop is left of BB
TEST_F(EngineTest, FirstToActPreflopIsLeftOfBigBlind)
{
    // 6 players hand
    initializeHandWithPlayers(6, gameData);

    // After initialize (before run loop), positions should be set in Player::resetForNewHand
    // Find BB and its left neighbor in acting order
    auto acting = m_hand->getActingPlayersList();
    auto bbIt = std::find_if(acting->begin(), acting->end(), [](const std::shared_ptr<Player>& p){
        return p->getPosition() == PlayerPosition::BigBlind;
    });
    ASSERT_NE(bbIt, acting->end()) << "BigBlind not found among acting players";

    auto nextIt = std::next(bbIt);
    if (nextIt == acting->end()) nextIt = acting->begin();

    auto expectedFirst = *nextIt;

    // Query engine helper for next to act at start of preflop
    auto nextToAct = getNextPlayerToActInRound(*m_hand, GameState::Preflop);
    ASSERT_NE(nextToAct, nullptr);

    EXPECT_EQ(nextToAct->getId(), expectedFirst->getId())
        << "First to act preflop must be left of BB";
}

} // namespace pkt::test
