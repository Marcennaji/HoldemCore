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

    m_events.clear();
    m_events.onBettingRoundStarted = [&](GameState state) { m_lastGameState = state; };
}

void PreflopStateTest::TearDown()
{
    EngineTest::TearDown();
}

TEST_F(PreflopStateTest, StartHandHeadsUpInitializePlayersCorrectly)
{
    initializeHandWithPlayers(2, gameData);

    auto playerSb = getPlayerById(m_actingPlayersList, 0);
    EXPECT_EQ(playerSb->getPosition(), PlayerPosition::ButtonSmallBlind);
    auto playerBb = getPlayerById(m_actingPlayersList, 1);
    EXPECT_EQ(playerBb->getPosition(), PlayerPosition::BigBlind);

    EXPECT_EQ(playerSb->getCash(), playerBb->getCash() + gameData.firstSmallBlind);
}
TEST_F(PreflopStateTest, StartHand3BotsInitializePlayersCorrectly)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    EXPECT_EQ(playerDealer->getPosition(), PlayerPosition::Button);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    EXPECT_EQ(playerSb->getPosition(), PlayerPosition::SmallBlind);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);
    EXPECT_EQ(playerBb->getPosition(), PlayerPosition::BigBlind);

    EXPECT_EQ(playerDealer->getCash(), playerBb->getCash() + gameData.firstSmallBlind * 2);
    EXPECT_EQ(playerSb->getCash(), playerBb->getCash() + gameData.firstSmallBlind);
}
TEST_F(PreflopStateTest, StartHand6BotsInitializePlayersCorrectly)
{
    initializeHandWithPlayers(6, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    EXPECT_EQ(playerDealer->getPosition(), PlayerPosition::Button);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    EXPECT_EQ(playerSb->getPosition(), PlayerPosition::SmallBlind);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);
    EXPECT_EQ(playerBb->getPosition(), PlayerPosition::BigBlind);
    auto playerUtg = getPlayerById(m_actingPlayersList, 3);
    EXPECT_EQ(playerUtg->getPosition(), PlayerPosition::UnderTheGun);
    auto playerMiddle = getPlayerById(m_actingPlayersList, 4);
    EXPECT_EQ(playerMiddle->getPosition(), PlayerPosition::Middle);
    auto playerCutoff = getPlayerById(m_actingPlayersList, 5);
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

    EXPECT_EQ(m_lastGameState, Preflop);
}
TEST_F(PreflopStateTest, OnlyCallsPreflopShouldGoToFlop)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    EXPECT_EQ(playerDealer->getPosition(), PlayerPosition::Button);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    EXPECT_EQ(playerSb->getPosition(), PlayerPosition::SmallBlind);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);
    EXPECT_EQ(playerBb->getPosition(), PlayerPosition::BigBlind);

    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Check});

    EXPECT_EQ(m_lastGameState, Flop);
}
TEST_F(PreflopStateTest, EverybodyFoldPreflopShouldGoToPostRiver)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Fold});

    EXPECT_EQ(m_lastGameState, PostRiver);
}

TEST_F(PreflopStateTest, OneRaiseKeepsBettingRoundOpen)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // step 1 : dealer fold
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    EXPECT_TRUE(isPlayerStillActive(playerSb->getId()));
    EXPECT_TRUE(isPlayerStillActive(playerBb->getId()));
    EXPECT_EQ(m_lastGameState, Preflop); // still in preflop

    // step 2 : small blind raises to 40 (min-raise on blind 20)
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Raise, 40});
    EXPECT_EQ(m_lastGameState, Preflop); // round not finished yet

    // step 3 : big blind calls 20
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Call});

    // now, the bets are equal → transition to Flop
    EXPECT_EQ(m_lastGameState, Flop);
}

TEST_F(PreflopStateTest, ReraiseUpdatesHighestBetAndKeepsRoundOpen)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Step 1: dealer folds
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    EXPECT_TRUE(isPlayerStillActive(playerSb->getId()));
    EXPECT_TRUE(isPlayerStillActive(playerBb->getId()));
    EXPECT_EQ(m_lastGameState, Preflop);

    // Step 2: small blind opens with a raise to 40
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Raise, 40});
    EXPECT_EQ(m_lastGameState, Preflop); // still in preflop

    // Step 3: big blind re-raises to 80
    m_hand->handlePlayerAction({playerBb->getId(), ActionType::Raise, 80});
    EXPECT_EQ(m_lastGameState, Preflop); // still in preflop after re-raise

    // Step 4: small blind calls the difference (40)
    m_hand->handlePlayerAction({playerSb->getId(), ActionType::Call});

    // now, the bets are equal → transition to Flop
    EXPECT_EQ(m_lastGameState, Flop);

    // Verify highest bet and last raiser
    auto bettingActions = m_hand->getBettingActions();
    EXPECT_EQ(bettingActions->getRoundHighestSet(), 0); // reset to 0 on the new round
    EXPECT_EQ(bettingActions->getPreflop().getLastRaiser()->getId(), playerBb->getId());
}

TEST_F(PreflopStateTest, RaiseBelowMinimumShouldBeRejected)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // Step 1: dealer folds
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Fold});
    EXPECT_TRUE(isPlayerStillActive(playerSb->getId()));
    EXPECT_TRUE(isPlayerStillActive(playerBb->getId()));
    EXPECT_EQ(m_lastGameState, Preflop);

    // Step 2: SB attempts an invalid raise (to less than BB + SB)
    int currentHighest = m_hand->getBettingActions()->getRoundHighestSet();
    int invalidRaise = currentHighest + (m_hand->getSmallBlind() / 2); // too small

    auto actionProcessor = m_hand->getActionProcessor();
    ASSERT_NE(actionProcessor, nullptr);

    bool actionAllowed =
        actionProcessor->isActionAllowed(*m_hand, {playerSb->getId(), ActionType::Raise, invalidRaise});

    EXPECT_FALSE(actionAllowed);

    // SB’s total bet must remain unchanged
    EXPECT_EQ(playerSb->getCurrentHandActions().getHandTotalBetAmount(), m_hand->getSmallBlind());

    // Current highest bet is still the BB
    EXPECT_EQ(m_hand->getBettingActions()->getRoundHighestSet(), m_hand->getSmallBlind() * 2);
}

TEST_F(PreflopStateTest, AllInInsteadOfRaiseIsAccepted)
{
    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(m_actingPlayersList, 0);
    auto playerSb = getPlayerById(m_actingPlayersList, 1);
    auto playerBb = getPlayerById(m_actingPlayersList, 2);

    // dealer goes allin
    m_hand->handlePlayerAction({playerDealer->getId(), ActionType::Allin});

    EXPECT_EQ(m_hand->getBettingActions()->getRoundHighestSet(), 1000);

    // dealer must have no chips left
    EXPECT_EQ(playerDealer->getCash(), 0);

    // Round is not yet closed — SB and BB still need to act
    EXPECT_EQ(m_lastGameState, Preflop);
}

} // namespace pkt::test
