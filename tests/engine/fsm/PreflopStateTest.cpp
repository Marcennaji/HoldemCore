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

    gameData.maxNumberOfPlayers = MAX_NUMBER_OF_PLAYERS;
    gameData.startMoney = 1000;
    gameData.firstSmallBlind = 10;
    gameData.tableProfile = TableProfile::RandomOpponents;
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

TEST_F(PreflopStateTest, StartHandHeadsUpInitializePlayersCorrectly)
{
    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    auto playerSb = getPlayerFsmById(myRunningPlayersListFsm, 0);
    EXPECT_EQ(playerSb->getButton(), ButtonSmallBlind);
    auto playerBb = getPlayerFsmById(myRunningPlayersListFsm, 1);
    EXPECT_EQ(playerBb->getButton(), ButtonBigBlind);

    EXPECT_EQ(playerSb->getLegacyPlayer()->getCash(),
              playerBb->getLegacyPlayer()->getCash() + gameData.firstSmallBlind);
}
TEST_F(PreflopStateTest, StartHand3BotsInitializePlayersCorrectly)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myRunningPlayersListFsm, 0);
    EXPECT_EQ(playerDealer->getButton(), ButtonDealer);
    auto playerSb = getPlayerFsmById(myRunningPlayersListFsm, 1);
    EXPECT_EQ(playerSb->getButton(), ButtonSmallBlind);
    auto playerBb = getPlayerFsmById(myRunningPlayersListFsm, 2);
    EXPECT_EQ(playerBb->getButton(), ButtonBigBlind);

    EXPECT_EQ(playerDealer->getLegacyPlayer()->getCash(),
              playerBb->getLegacyPlayer()->getCash() + gameData.firstSmallBlind * 2);
    EXPECT_EQ(playerSb->getLegacyPlayer()->getCash(),
              playerBb->getLegacyPlayer()->getCash() + gameData.firstSmallBlind);
}
TEST_F(PreflopStateTest, StartHand6BotsInitializePlayersCorrectly)
{
    initializeHandFsmForTesting(6, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myRunningPlayersListFsm, 0);
    EXPECT_EQ(playerDealer->getButton(), ButtonDealer);
    auto playerSb = getPlayerFsmById(myRunningPlayersListFsm, 1);
    EXPECT_EQ(playerSb->getButton(), ButtonSmallBlind);
    auto playerBb = getPlayerFsmById(myRunningPlayersListFsm, 2);
    EXPECT_EQ(playerBb->getButton(), ButtonBigBlind);
    auto playerUtg = getPlayerFsmById(myRunningPlayersListFsm, 3);
    EXPECT_EQ(playerUtg->getButton(), ButtonNone);
    auto playerUtgPlusOne = getPlayerFsmById(myRunningPlayersListFsm, 4);
    EXPECT_EQ(playerUtg->getButton(), ButtonNone);
    auto playerUtgPlusTwo = getPlayerFsmById(myRunningPlayersListFsm, 5);
    EXPECT_EQ(playerUtgPlusTwo->getButton(), ButtonNone);

    EXPECT_EQ(playerDealer->getLegacyPlayer()->getCash(),
              playerBb->getLegacyPlayer()->getCash() + gameData.firstSmallBlind * 2);
    EXPECT_EQ(playerSb->getLegacyPlayer()->getCash(),
              playerBb->getLegacyPlayer()->getCash() + gameData.firstSmallBlind);
    EXPECT_EQ(playerUtg->getLegacyPlayer()->getCash(),
              playerBb->getLegacyPlayer()->getCash() + gameData.firstSmallBlind * 2);
    EXPECT_EQ(playerUtgPlusOne->getLegacyPlayer()->getCash(),
              playerBb->getLegacyPlayer()->getCash() + gameData.firstSmallBlind * 2);
    EXPECT_EQ(playerUtgPlusTwo->getLegacyPlayer()->getCash(),
              playerBb->getLegacyPlayer()->getCash() + gameData.firstSmallBlind * 2);
}
TEST_F(PreflopStateTest, StartHandShouldGoToPreflop)
{
    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    EXPECT_EQ(myLastGameState, GameStatePreflop);
}
TEST_F(PreflopStateTest, DISABLED_EverybodyCallPreflopShouldGoToFlop)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myRunningPlayersListFsm, 0);
    EXPECT_EQ(playerDealer->getButton(), ButtonDealer);
    auto playerSb = getPlayerFsmById(myRunningPlayersListFsm, 1);
    EXPECT_EQ(playerSb->getButton(), ButtonSmallBlind);
    auto playerBb = getPlayerFsmById(myRunningPlayersListFsm, 2);
    EXPECT_EQ(playerBb->getButton(), ButtonBigBlind);

    myHandFsm->processPlayerAction({0, ActionType::Call, 20});
    myHandFsm->processPlayerAction({1, ActionType::Call, 10});
    myHandFsm->processPlayerAction({2, ActionType::Check});

    EXPECT_EQ(myLastGameState, GameStateFlop);
}
TEST_F(PreflopStateTest, DISABLED_EverybodyFoldPreflopShouldGoToPostRiver)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();
    myHandFsm->processPlayerAction({0, ActionType::Fold});
    myHandFsm->processPlayerAction({1, ActionType::Fold});

    EXPECT_EQ(myLastGameState, GameStatePostRiver);
}
} // namespace pkt::test
