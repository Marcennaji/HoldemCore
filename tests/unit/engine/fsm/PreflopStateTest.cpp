// tests/PreflopState.cpp

#include "PreflopStateTest.h"
#include <iostream>
#include "core/engine/model/PlayerAction.h"
#include "core/engine/state/PreflopState.h"
#include "core/player/Helpers.h"

using namespace pkt::core;
using namespace pkt::core::player;
using namespace std;

namespace pkt::test
{
void PreflopStateTest::logTestMessage(const std::string& message) const
{
    getLogger().info("PreflopState : " + message);
}

void PreflopStateTest::SetUp()
{
    EngineTest::SetUp();

    myEvents.clear();
    myEvents.onBettingRoundStarted = [&](GameState state) { myLastGameState = state; };
}

void PreflopStateTest::TearDown()
{
    EngineTest::TearDown();
}

TEST_F(PreflopStateTest, StartHandHeadsUpInitializePlayersCorrectly)
{
    initializeHandWithPlayers(2, gameData);

    auto playerSb = getPlayerById(myActingPlayersList, 0);
    EXPECT_EQ(playerSb->getPosition(), PlayerPosition::ButtonSmallBlind);
    auto playerBb = getPlayerById(myActingPlayersList, 1);
    EXPECT_EQ(playerBb->getPosition(), PlayerPosition::BigBlind);

    EXPECT_EQ(playerSb->getCash(), playerBb->getCash() + gameData.firstSmallBlind);
}
TEST_F(PreflopStateTest, StartHand3BotsInitializePlayersCorrectly)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    EXPECT_EQ(playerDealer->getPosition(), PlayerPosition::Button);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    EXPECT_EQ(playerSb->getPosition(), PlayerPosition::SmallBlind);
    auto playerBb = getPlayerById(myActingPlayersList, 2);
    EXPECT_EQ(playerBb->getPosition(), PlayerPosition::BigBlind);

    EXPECT_EQ(playerDealer->getCash(), playerBb->getCash() + gameData.firstSmallBlind * 2);
    EXPECT_EQ(playerSb->getCash(), playerBb->getCash() + gameData.firstSmallBlind);
}
TEST_F(PreflopStateTest, StartHand6BotsInitializePlayersCorrectly)
{
    initializeHandWithPlayers(6, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    EXPECT_EQ(playerDealer->getPosition(), PlayerPosition::Button);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    EXPECT_EQ(playerSb->getPosition(), PlayerPosition::SmallBlind);
    auto playerBb = getPlayerById(myActingPlayersList, 2);
    EXPECT_EQ(playerBb->getPosition(), PlayerPosition::BigBlind);
    auto playerUtg = getPlayerById(myActingPlayersList, 3);
    EXPECT_EQ(playerUtg->getPosition(), PlayerPosition::UnderTheGun);
    auto playerMiddle = getPlayerById(myActingPlayersList, 4);
    EXPECT_EQ(playerMiddle->getPosition(), PlayerPosition::Middle);
    auto playerCutoff = getPlayerById(myActingPlayersList, 5);
    EXPECT_EQ(playerCutoff->getPosition(), PlayerPosition::Cutoff);

    EXPECT_EQ(playerDealer->getCash(), playerBb->getCash() + gameData.firstSmallBlind * 2);
    EXPECT_EQ(playerSb->getCash(), playerBb->getCash() + gameData.firstSmallBlind);
    EXPECT_EQ(playerUtg->getCash(), playerBb->getCash() + gameData.firstSmallBlind * 2);
    EXPECT_EQ(playerMiddle->getCash(), playerBb->getCash() + gameData.firstSmallBlind * 2);
    EXPECT_EQ(playerCutoff->getCash(), playerBb->getCash() + gameData.firstSmallBlind * 2);
}
TEST_F(PreflopStateTest, StartHandShouldGoToPreflop)
{
    initializeHandWithPlayers(2, gameData);

    EXPECT_EQ(myLastGameState, Preflop);
}
TEST_F(PreflopStateTest, OnlyCallsPreflopShouldGoToFlop)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    EXPECT_EQ(playerDealer->getPosition(), PlayerPosition::Button);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    EXPECT_EQ(playerSb->getPosition(), PlayerPosition::SmallBlind);
    auto playerBb = getPlayerById(myActingPlayersList, 2);
    EXPECT_EQ(playerBb->getPosition(), PlayerPosition::BigBlind);

    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(myLastGameState, Flop);
}
TEST_F(PreflopStateTest, EverybodyFoldPreflopShouldGoToPostRiver)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Fold});

    EXPECT_EQ(myLastGameState, PostRiver);
}

TEST_F(PreflopStateTest, OneRaiseKeepsBettingRoundOpen)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // step 1 : dealer fold
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    EXPECT_TRUE(isPlayerStillActive(playerSb->getId()));
    EXPECT_TRUE(isPlayerStillActive(playerBb->getId()));
    EXPECT_EQ(myLastGameState, Preflop); // still in preflop

    // step 2 : small blind raises to 40 (min-raise on blind 20)
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Raise, 40});
    EXPECT_EQ(myLastGameState, Preflop); // round not finished yet

    // step 3 : big blind calls 20
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Call});

    // now, the bets are equal → transition to Flop
    EXPECT_EQ(myLastGameState, Flop);
}

TEST_F(PreflopStateTest, ReraiseUpdatesHighestBetAndKeepsRoundOpen)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Step 1: dealer folds
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    EXPECT_TRUE(isPlayerStillActive(playerSb->getId()));
    EXPECT_TRUE(isPlayerStillActive(playerBb->getId()));
    EXPECT_EQ(myLastGameState, Preflop);

    // Step 2: small blind opens with a raise to 40
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Raise, 40});
    EXPECT_EQ(myLastGameState, Preflop); // still in preflop

    // Step 3: big blind re-raises to 80
    myHand->handlePlayerAction({playerBb->getId(), ActionType::Raise, 80});
    EXPECT_EQ(myLastGameState, Preflop); // still in preflop after re-raise

    // Step 4: small blind calls the difference (40)
    myHand->handlePlayerAction({playerSb->getId(), ActionType::Call});

    // now, the bets are equal → transition to Flop
    EXPECT_EQ(myLastGameState, Flop);

    // Verify highest bet and last raiser
    auto bettingActions = myHand->getBettingActions();
    EXPECT_EQ(bettingActions->getRoundHighestSet(), 0); // reset to 0 on the new round
    EXPECT_EQ(bettingActions->getPreflop().getLastRaiserId(), playerBb->getId());
}

TEST_F(PreflopStateTest, RaiseBelowMinimumShouldBeRejected)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // Step 1: dealer folds
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    EXPECT_TRUE(isPlayerStillActive(playerSb->getId()));
    EXPECT_TRUE(isPlayerStillActive(playerBb->getId()));
    EXPECT_EQ(myLastGameState, Preflop);

    // Step 2: SB attempts an invalid raise (to less than BB + SB)
    int currentHighest = myHand->getBettingActions()->getRoundHighestSet();
    int invalidRaise = currentHighest + (myHand->getSmallBlind() / 2); // too small

    auto actionProcessor = myHand->getActionProcessor();
    ASSERT_NE(actionProcessor, nullptr);

    bool actionAllowed =
        actionProcessor->isActionAllowed(*myHand, {playerSb->getId(), ActionType::Raise, invalidRaise});

    EXPECT_FALSE(actionAllowed);

    // SB’s total bet must remain unchanged
    EXPECT_EQ(playerSb->getCurrentHandActions().getHandTotalBetAmount(), myHand->getSmallBlind());

    // Current highest bet is still the BB
    EXPECT_EQ(myHand->getBettingActions()->getRoundHighestSet(), myHand->getSmallBlind() * 2);
}

TEST_F(PreflopStateTest, AllInInsteadOfRaiseIsAccepted)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

    // dealer goes allin
    myHand->handlePlayerAction({playerDealer->getId(), ActionType::Allin});

    EXPECT_EQ(myHand->getBettingActions()->getRoundHighestSet(), 1000);

    // dealer must have no chips left
    EXPECT_EQ(playerDealer->getCash(), 0);

    // Round is not yet closed — SB and BB still need to act
    EXPECT_EQ(myLastGameState, Preflop);
}

} // namespace pkt::test
