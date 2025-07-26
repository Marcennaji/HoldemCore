// tests/PreflopState.cpp

#include "PreflopStateTest.h"
#include "core/engine/model/PlayerAction.h"
#include "core/engine/round_states/PreflopState.h"
#include "core/player/Helpers.h"
#include "core/services/GlobalServices.h"

using namespace pkt::core;
using namespace pkt::core::player;
using namespace std;

namespace pkt::test
{
void PreflopStateTest::logTestMessage(const std::string& message) const
{
    GlobalServices::instance().logger()->verbose("PreflopState : " + message);
}

void PreflopStateTest::SetUp()
{
    EngineTest::SetUp();
    myEvents.clear();
    myEvents.onBettingRoundStarted = [&](GameState state) { myLastGameState = state; };
}

bool PreflopStateTest::isPlayerStillActive(unsigned id) const
{
    for (const auto& p : *myHandFsm->getRunningPlayersList())
    {
        if (p->getLegacyPlayer()->getId() == id)
            return true;
    }
    return false;
}
void PreflopStateTest::TearDown()
{
}

TEST_F(PreflopStateTest, StartShouldGoToPreflop)
{
    initializeHandFsmForTesting(2);
    myHandFsm->start();
    EXPECT_EQ(myLastGameState, GameStatePreflop);
}
TEST_F(PreflopStateTest, EverybodyCallShouldGoToFlop)
{
    initializeHandFsmForTesting(3);
    myHandFsm->start();
    myHandFsm->processPlayerAction({0, ActionType::Call});
    myHandFsm->processPlayerAction({1, ActionType::Call});
    myHandFsm->processPlayerAction({2, ActionType::Check});

    EXPECT_EQ(myLastGameState, GameStateFlop);
}
TEST_F(PreflopStateTest, EverybodyFoldShouldGoToPostRiver)
{
    initializeHandFsmForTesting(3);
    myHandFsm->start();
    myHandFsm->processPlayerAction({0, ActionType::Fold});
    myHandFsm->processPlayerAction({1, ActionType::Fold});

    EXPECT_EQ(myLastGameState, GameStatePostRiver);
}
} // namespace pkt::test
