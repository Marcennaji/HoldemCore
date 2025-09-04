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
    GlobalServices::instance().logger().verbose("PreflopState : " + message);
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
        if (p->getId() == id)
            return true;
    }
    return false;
}
void PreflopStateTest::TearDown()
{
    EngineTest::TearDown();
}

TEST_F(PreflopStateTest, StartHandHeadsUpInitializePlayersCorrectly)
{
    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    auto playerSb = getPlayerFsmById(myRunningPlayersListFsm, 0);
    EXPECT_EQ(playerSb->getPosition(), PlayerPosition::ButtonSmallBlind);
    auto playerBb = getPlayerFsmById(myRunningPlayersListFsm, 1);
    EXPECT_EQ(playerBb->getPosition(), PlayerPosition::BigBlind);

    EXPECT_EQ(playerSb->getCash(), playerBb->getCash() + gameData.firstSmallBlind);
}
TEST_F(PreflopStateTest, StartHand3BotsInitializePlayersCorrectly)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myRunningPlayersListFsm, 0);
    EXPECT_EQ(playerDealer->getPosition(), PlayerPosition::Button);
    auto playerSb = getPlayerFsmById(myRunningPlayersListFsm, 1);
    EXPECT_EQ(playerSb->getPosition(), PlayerPosition::SmallBlind);
    auto playerBb = getPlayerFsmById(myRunningPlayersListFsm, 2);
    EXPECT_EQ(playerBb->getPosition(), PlayerPosition::BigBlind);

    EXPECT_EQ(playerDealer->getCash(), playerBb->getCash() + gameData.firstSmallBlind * 2);
    EXPECT_EQ(playerSb->getCash(), playerBb->getCash() + gameData.firstSmallBlind);
}
TEST_F(PreflopStateTest, StartHand6BotsInitializePlayersCorrectly)
{
    initializeHandFsmForTesting(6, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myRunningPlayersListFsm, 0);
    EXPECT_EQ(playerDealer->getPosition(), PlayerPosition::Button);
    auto playerSb = getPlayerFsmById(myRunningPlayersListFsm, 1);
    EXPECT_EQ(playerSb->getPosition(), PlayerPosition::SmallBlind);
    auto playerBb = getPlayerFsmById(myRunningPlayersListFsm, 2);
    EXPECT_EQ(playerBb->getPosition(), PlayerPosition::BigBlind);
    auto playerUtg = getPlayerFsmById(myRunningPlayersListFsm, 3);
    EXPECT_EQ(playerUtg->getPosition(), PlayerPosition::UnderTheGun);
    auto playerMiddle = getPlayerFsmById(myRunningPlayersListFsm, 4);
    EXPECT_EQ(playerMiddle->getPosition(), PlayerPosition::Middle);
    auto playerCutoff = getPlayerFsmById(myRunningPlayersListFsm, 5);
    EXPECT_EQ(playerCutoff->getPosition(), PlayerPosition::Cutoff);

    EXPECT_EQ(playerDealer->getCash(), playerBb->getCash() + gameData.firstSmallBlind * 2);
    EXPECT_EQ(playerSb->getCash(), playerBb->getCash() + gameData.firstSmallBlind);
    EXPECT_EQ(playerUtg->getCash(), playerBb->getCash() + gameData.firstSmallBlind * 2);
    EXPECT_EQ(playerMiddle->getCash(), playerBb->getCash() + gameData.firstSmallBlind * 2);
    EXPECT_EQ(playerCutoff->getCash(), playerBb->getCash() + gameData.firstSmallBlind * 2);
}
TEST_F(PreflopStateTest, StartHandShouldGoToPreflop)
{
    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    EXPECT_EQ(myLastGameState, Preflop);
}
TEST_F(PreflopStateTest, OnlyCallsPreflopShouldGoToFlop)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myRunningPlayersListFsm, 0);
    EXPECT_EQ(playerDealer->getPosition(), PlayerPosition::Button);
    auto playerSb = getPlayerFsmById(myRunningPlayersListFsm, 1);
    EXPECT_EQ(playerSb->getPosition(), PlayerPosition::SmallBlind);
    auto playerBb = getPlayerFsmById(myRunningPlayersListFsm, 2);
    EXPECT_EQ(playerBb->getPosition(), PlayerPosition::BigBlind);

    myHandFsm->handlePlayerAction({0, ActionType::Call, 20});
    myHandFsm->handlePlayerAction({1, ActionType::Call, 10});
    myHandFsm->handlePlayerAction({2, ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);
}
TEST_F(PreflopStateTest, EverybodyFoldPreflopShouldGoToPostRiver)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();
    myHandFsm->handlePlayerAction({0, ActionType::Fold});
    myHandFsm->handlePlayerAction({1, ActionType::Fold});

    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(PreflopStateTest, OneRaiseKeepsBettingRoundOpen)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myRunningPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myRunningPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myRunningPlayersListFsm, 2);

    // step 1 : dealer fold
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    EXPECT_TRUE(isPlayerStillActive(playerSb->getId()));
    EXPECT_TRUE(isPlayerStillActive(playerBb->getId()));
    EXPECT_EQ(myLastGameState, Preflop); // still in preflop

    // step 2 : small blind raises to 40 (min-raise on blind 20)
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Raise, 40});
    EXPECT_EQ(myLastGameState, Preflop); // round not finished yet

    // step 3 : big blind calls 20
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Call, 20});

    // now, the bets are equal → transition to Flop
    EXPECT_EQ(myLastGameState, Flop);
}

TEST_F(PreflopStateTest, ReraiseUpdatesHighestBetAndKeepsRoundOpen)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myRunningPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myRunningPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myRunningPlayersListFsm, 2);

    // Step 1: dealer folds
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    EXPECT_TRUE(isPlayerStillActive(playerSb->getId()));
    EXPECT_TRUE(isPlayerStillActive(playerBb->getId()));
    EXPECT_EQ(myLastGameState, Preflop);

    // Step 2: small blind opens with a raise to 40
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Raise, 40});
    EXPECT_EQ(myLastGameState, Preflop); // still in preflop

    // Step 3: big blind re-raises to 80
    myHandFsm->handlePlayerAction({playerBb->getId(), ActionType::Raise, 80});
    EXPECT_EQ(myLastGameState, Preflop); // still in preflop after re-raise

    // Step 4: small blind calls the difference (40)
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call, 40});

    // now, the bets are equal → transition to Flop
    EXPECT_EQ(myLastGameState, Flop);

    // Verify highest bet and last raiser
    auto bettingActions = myHandFsm->getBettingActions();
    EXPECT_EQ(bettingActions->getHighestSet(), 0); // reset to 0 on the new round
    EXPECT_EQ(bettingActions->getPreflop().getLastRaiserId(), playerBb->getId());
}

TEST_F(PreflopStateTest, RaiseBelowMinimumShouldBeRejected)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myRunningPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myRunningPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myRunningPlayersListFsm, 2);

    // Step 1: dealer folds
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    EXPECT_TRUE(isPlayerStillActive(playerSb->getId()));
    EXPECT_TRUE(isPlayerStillActive(playerBb->getId()));
    EXPECT_EQ(myLastGameState, Preflop);

    // Step 2: SB attempts an invalid raise (to less than BB + SB)
    int currentHighest = myHandFsm->getBettingActions()->getHighestSet();
    int invalidRaise = currentHighest + (myHandFsm->getSmallBlind() / 2); // too small

    auto actionProcessor = myHandFsm->getActionProcessor();
    ASSERT_NE(actionProcessor, nullptr);

    bool actionAllowed =
        actionProcessor->isActionAllowed(*myHandFsm, {playerSb->getId(), ActionType::Raise, invalidRaise});

    EXPECT_FALSE(actionAllowed);

    // SB’s total bet must remain unchanged
    EXPECT_EQ(playerSb->getTotalBetAmount(), myHandFsm->getSmallBlind());

    // Current highest bet is still the BB
    EXPECT_EQ(myHandFsm->getBettingActions()->getHighestSet(), myHandFsm->getSmallBlind() * 2);
}

TEST_F(PreflopStateTest, AllInInsteadOfRaiseIsAccepted)
{
    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    auto playerDealer = getPlayerFsmById(myRunningPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myRunningPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myRunningPlayersListFsm, 2);

    // dealer goes allin
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Allin});

    EXPECT_EQ(myHandFsm->getBettingActions()->getHighestSet(), 1000);

    // dealer must have no chips left
    EXPECT_EQ(playerDealer->getCash(), 0);

    // Round is not yet closed — SB and BB still need to act
    EXPECT_EQ(myLastGameState, Preflop);
}

} // namespace pkt::test
