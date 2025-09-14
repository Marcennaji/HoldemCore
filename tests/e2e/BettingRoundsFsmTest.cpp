// tests/BettingRoundsFsmTest.cpp

#include "BettingRoundsFsmTest.h"
#include "common/DeterministicStrategy.h"
#include "core/engine/Helpers.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/Helpers.h"
#include "core/services/GlobalServices.h"

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

void BettingRoundsFsmTest::logTestMessage(const std::string& message) const
{
    GlobalServices::instance().logger().verbose("BettingRoundsFsmTest : " + message);
}

void BettingRoundsFsmTest::SetUp()
{
    EngineTest::SetUp();
    myEvents.clear();
    stateSequence.clear();

    myEvents.onBettingRoundStarted = [&](GameState state)
    {
        myLastGameState = state;
        stateSequence.push_back(state);
    };
}

void BettingRoundsFsmTest::TearDown()
{
    checkPostRiverConditions();
    checkStateTransitions();
}

bool BettingRoundsFsmTest::isPlayerStillActive(unsigned id) const
{
    for (const auto& p : *myHandFsm->getActingPlayersList())
    {
        if (p->getId() == id)
            return true;
    }
    return false;
}
void BettingRoundsFsmTest::checkStateTransitions()
{
    // Verify we have a valid sequence
    EXPECT_FALSE(stateSequence.empty()) << "Should have recorded state transitions";

    // Verify PostRiver is the final state
    EXPECT_EQ(stateSequence.back(), PostRiver)
        << "Final state should be PostRiver, found " + gameStateToString(stateSequence.back());

    // Verify no invalid state transitions
    for (size_t i = 1; i < stateSequence.size(); ++i)
    {
        GameState prev = stateSequence[i - 1];
        GameState curr = stateSequence[i];

        // Check for valid transitions
        bool validTransition = (prev == Preflop && (curr == Flop || curr == PostRiver)) ||
                               (prev == Flop && (curr == Turn || curr == PostRiver)) ||
                               (prev == Turn && (curr == River || curr == PostRiver)) ||
                               (prev == River && curr == PostRiver);

        EXPECT_TRUE(validTransition) << "Invalid transition from " << gameStateToString(prev) << " to "
                                     << gameStateToString(curr);
    }
}

// Tests for FSM betting rounds and transitions

TEST_F(BettingRoundsFsmTest, StartShouldGoFromPreflopToPostRiverHeadsUp)
{
    logTestMessage("Testing heads-up hand completion");

    initializeHandFsmWithPlayers(2, gameData);

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Inject deterministic strategies - SB folds, BB wins
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerSb->getId(), pkt::core::ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    // Big blind does nothing here; we don't configure Preflop action
    playerBb->setStrategy(std::move(bbStrategy));

    myHandFsm->runGameLoop(); // Automatically prompts players and drives game to completion

    // Verify we reached PostRiver state
    EXPECT_EQ(myLastGameState, PostRiver) << "Hand should complete and reach PostRiver state";

    // Verify final state is terminal
    EXPECT_TRUE(myHandFsm->getState().isTerminal()) << "Final state should be terminal";
}

TEST_F(BettingRoundsFsmTest, StartShouldGoFromPreflopToPostRiver3Players)
{
    logTestMessage("Testing 3-player hand completion");

    initializeHandFsmWithPlayers(3, gameData);

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Inject deterministic strategies
    auto dealerStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    dealerStrategy->setLastAction(pkt::core::GameState::Preflop, {playerDealer->getId(), pkt::core::ActionType::Fold});
    playerDealer->setStrategy(std::move(dealerStrategy));

    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerSb->getId(), pkt::core::ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    // Big blind does nothing here; we don’t configure Preflop action
    playerBb->setStrategy(std::move(bbStrategy));

    myHandFsm->runGameLoop(); // Automatically prompts players and drives game to completion

    // Verify we reached PostRiver state
    EXPECT_EQ(myLastGameState, PostRiver) << "3-player hand should complete and reach PostRiver state";
}

TEST_F(BettingRoundsFsmTest, StartShouldGoFromPreflopToPostRiver6Players)
{
    logTestMessage("Testing 6-player hand completion");

    initializeHandFsmWithPlayers(6, gameData);

    auto player0 = getPlayerFsmById(myActingPlayersListFsm, 0); // Dealer
    auto player1 = getPlayerFsmById(myActingPlayersListFsm, 1); // SB
    auto player2 = getPlayerFsmById(myActingPlayersListFsm, 2); // BB
    auto player3 = getPlayerFsmById(myActingPlayersListFsm, 3); // UTG
    auto player4 = getPlayerFsmById(myActingPlayersListFsm, 4); // MP
    auto player5 = getPlayerFsmById(myActingPlayersListFsm, 5); // CO

    // Inject deterministic strategies - most players fold, BB wins
    auto dealerStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    dealerStrategy->setLastAction(pkt::core::GameState::Preflop, {player0->getId(), pkt::core::ActionType::Fold});
    player0->setStrategy(std::move(dealerStrategy));

    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {player1->getId(), pkt::core::ActionType::Fold});
    player1->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    // Big blind does nothing here; we don't configure Preflop action
    player2->setStrategy(std::move(bbStrategy));

    auto utgStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    utgStrategy->setLastAction(pkt::core::GameState::Preflop, {player3->getId(), pkt::core::ActionType::Fold});
    player3->setStrategy(std::move(utgStrategy));

    auto mpStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    mpStrategy->setLastAction(pkt::core::GameState::Preflop, {player4->getId(), pkt::core::ActionType::Fold});
    player4->setStrategy(std::move(mpStrategy));

    auto coStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    coStrategy->setLastAction(pkt::core::GameState::Preflop, {player5->getId(), pkt::core::ActionType::Fold});
    player5->setStrategy(std::move(coStrategy));

    myHandFsm->runGameLoop(); // Automatically prompts players and drives game to completion

    // Verify we reached PostRiver state
    EXPECT_EQ(myLastGameState, PostRiver) << "6-player hand should complete and reach PostRiver state";

    // Verify final state is terminal
    EXPECT_TRUE(myHandFsm->getState().isTerminal()) << "Final state should be terminal";
}

TEST_F(BettingRoundsFsmTest, AllInPlayersTransitionToPostRiverCorrectly)
{
    logTestMessage("Testing all-in scenario transitions to PostRiver");

    initializeHandFsmWithPlayers(2, gameData);

    // Set up one player with limited cash to force all-in
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Reduce SB player's cash to force all-in scenario
    playerSb->setCash(50); // After posting SB (10), they'll have 40 left

    // Inject deterministic strategies - SB goes all-in, BB calls
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerSb->getId(), pkt::core::ActionType::Allin});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    bbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerBb->getId(), pkt::core::ActionType::Call});
    playerBb->setStrategy(std::move(bbStrategy));

    myHandFsm->runGameLoop(); // Automatically prompts players and drives game to completion

    EXPECT_EQ(stateSequence.size(), 2)
        << "This preflop All-in scenario should have only 2 states: Preflop and PostRiver";
    EXPECT_EQ(stateSequence[0], Preflop) << "First state should be Preflop";
    EXPECT_EQ(stateSequence[1], PostRiver) << "Second state should be PostRiver";
}

TEST_F(BettingRoundsFsmTest, PlayersDoNotActAfterFolding)
{
    logTestMessage("Testing that folded players don't act in later rounds");

    initializeHandFsmWithPlayers(4, gameData);

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);
    auto playerUtg = getPlayerFsmById(myActingPlayersListFsm, 3);

    // Setup strategies - Dealer and UTG fold, SB folds, BB wins
    auto dealerStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    dealerStrategy->setLastAction(pkt::core::GameState::Preflop, {playerDealer->getId(), pkt::core::ActionType::Fold});
    playerDealer->setStrategy(std::move(dealerStrategy));

    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerSb->getId(), pkt::core::ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    // BB wins when others fold
    playerBb->setStrategy(std::move(bbStrategy));

    auto utgStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    utgStrategy->setLastAction(pkt::core::GameState::Preflop, {playerUtg->getId(), pkt::core::ActionType::Fold});
    playerUtg->setStrategy(std::move(utgStrategy));

    myHandFsm->runGameLoop();

    EXPECT_EQ(myLastGameState, PostRiver) << "Hand should complete after folds";

    // Verify folded players are not in final acting list
    EXPECT_FALSE(isPlayerStillActive(playerDealer->getId())) << "Dealer should have folded and not be active";
    EXPECT_FALSE(isPlayerStillActive(playerSb->getId())) << "SB should have folded and not be active";
    EXPECT_FALSE(isPlayerStillActive(playerUtg->getId())) << "UTG should have folded and not be active";

    // Use hand action history to verify folded players don't act after folding
    const auto& handHistory = myHandFsm->getHandActionHistory();
    EXPECT_FALSE(handHistory.empty()) << "Hand action history should not be empty";

    // Count fold actions in hand history - should match our expected folds
    int foldCount = 0;
    for (const auto& roundHistory : handHistory)
    {
        for (const auto& [playerId, action] : roundHistory.actions)
        {
            if (action == ActionType::Fold)
            {
                foldCount++;
                // Verify the fold is from one of our expected players
                EXPECT_TRUE(playerId == playerDealer->getId() || playerId == playerSb->getId() ||
                            playerId == playerUtg->getId())
                    << "Fold action from unexpected player " << playerId;
            }
        }
    }
    EXPECT_EQ(foldCount, 3) << "Should have exactly 3 fold actions recorded";
}

// TODO: Add more tests following the BettingRoundsLegacyTest pattern:
TEST_F(BettingRoundsFsmTest, ShouldRecordAllActionsInHandHistoryChronologically)
{
    logTestMessage("Testing that all player actions are recorded chronologically in hand history");

    initializeHandFsmWithPlayers(3, gameData);

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Setup deterministic strategies - specific actions for verification
    auto dealerStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    dealerStrategy->setLastAction(pkt::core::GameState::Preflop, {playerDealer->getId(), pkt::core::ActionType::Fold});
    playerDealer->setStrategy(std::move(dealerStrategy));

    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerSb->getId(), pkt::core::ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    // BB wins when others fold
    playerBb->setStrategy(std::move(bbStrategy));

    myHandFsm->runGameLoop();

    // Verify the hand action history contains exactly what we expect
    const auto& handHistory = myHandFsm->getHandActionHistory();
    EXPECT_FALSE(handHistory.empty()) << "Hand action history should not be empty";

    // Find preflop round
    auto preflopRound = std::find_if(handHistory.begin(), handHistory.end(),
                                     [](const auto& round) { return round.round == GameState::Preflop; });
    ASSERT_NE(preflopRound, handHistory.end()) << "Should have preflop round recorded";

    const auto& preflopActions = preflopRound->actions;
    EXPECT_EQ(preflopActions.size(), 4u) << "Should have exactly 4 actions: PostSB, PostBB, Dealer fold, SB fold";

    // Verify chronological order of expected actions
    EXPECT_EQ(preflopActions[0].first, playerSb->getId()) << "First action should be SB posting blind";
    EXPECT_EQ(preflopActions[0].second, ActionType::PostSmallBlind) << "First action should be PostSmallBlind";

    EXPECT_EQ(preflopActions[1].first, playerBb->getId()) << "Second action should be BB posting blind";
    EXPECT_EQ(preflopActions[1].second, ActionType::PostBigBlind) << "Second action should be PostBigBlind";

    EXPECT_EQ(preflopActions[2].first, playerDealer->getId()) << "Third action should be Dealer decision";
    EXPECT_EQ(preflopActions[2].second, ActionType::Fold) << "Third action should be Fold";

    EXPECT_EQ(preflopActions[3].first, playerSb->getId()) << "Fourth action should be SB decision";
    EXPECT_EQ(preflopActions[3].second, ActionType::Fold) << "Fourth action should be Fold";
}

TEST_F(BettingRoundsFsmTest, ActionOrderStartsCorrectlyInHeadsUpPreflop)
{
    logTestMessage("Testing that action order starts correctly in heads-up preflop");

    initializeHandFsmWithPlayers(2, gameData);

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Setup strategies - SB folds to avoid infinite loop, we just need to verify the action order
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerSb->getId(), pkt::core::ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    // BB doesn't need to act if SB folds
    playerBb->setStrategy(std::move(bbStrategy));

    myHandFsm->runGameLoop();

    // Verify the hand action history shows correct action order
    const auto& handHistory = myHandFsm->getHandActionHistory();
    EXPECT_FALSE(handHistory.empty()) << "Hand action history should not be empty";

    // Find preflop round
    auto preflopRound = std::find_if(handHistory.begin(), handHistory.end(),
                                     [](const auto& round) { return round.round == GameState::Preflop; });
    ASSERT_NE(preflopRound, handHistory.end()) << "Should have preflop round recorded";

    const auto& preflopActions = preflopRound->actions;
    EXPECT_EQ(preflopActions.size(), 3u) << "Should have exactly 3 actions: PostSB, PostBB, SB fold";

    // Verify correct heads-up preflop action order
    EXPECT_EQ(preflopActions[0].first, playerSb->getId()) << "First action should be SB posting blind";
    EXPECT_EQ(preflopActions[0].second, ActionType::PostSmallBlind) << "First action should be PostSmallBlind";

    EXPECT_EQ(preflopActions[1].first, playerBb->getId()) << "Second action should be BB posting blind";
    EXPECT_EQ(preflopActions[1].second, ActionType::PostBigBlind) << "Second action should be PostBigBlind";

    // In heads-up preflop, SB acts first (after blinds are posted)
    EXPECT_EQ(preflopActions[2].first, playerSb->getId())
        << "Third action should be SB decision (first to act in heads-up preflop)";
    EXPECT_EQ(preflopActions[2].second, ActionType::Fold) << "Third action should be Fold";
}

TEST_F(BettingRoundsFsmTest, ValidActionsAreCorrectlyDetermined)
{
    logTestMessage("Testing that valid actions are correctly determined for players");

    initializeHandFsmWithPlayers(2, gameData);

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Check valid actions for SB after blinds are posted (after initialization)
    auto validActionsSb = getValidActionsForPlayer(*myHandFsm, playerSb->getId());
    EXPECT_FALSE(validActionsSb.empty()) << "SB should have valid actions available";

    // SB should be able to fold, call (to match BB), or raise
    EXPECT_TRUE(std::find(validActionsSb.begin(), validActionsSb.end(), ActionType::Fold) != validActionsSb.end())
        << "SB should be able to fold";
    EXPECT_TRUE(std::find(validActionsSb.begin(), validActionsSb.end(), ActionType::Call) != validActionsSb.end())
        << "SB should be able to call";
    EXPECT_TRUE(std::find(validActionsSb.begin(), validActionsSb.end(), ActionType::Raise) != validActionsSb.end())
        << "SB should be able to raise";
    EXPECT_TRUE(std::find(validActionsSb.begin(), validActionsSb.end(), ActionType::Allin) != validActionsSb.end())
        << "SB should be able to go all-in";

    // SB should NOT be able to check (since there's a bet to call) or bet (since BB already posted)
    EXPECT_TRUE(std::find(validActionsSb.begin(), validActionsSb.end(), ActionType::Check) == validActionsSb.end())
        << "SB should NOT be able to check when there's a bet to call";
    EXPECT_TRUE(std::find(validActionsSb.begin(), validActionsSb.end(), ActionType::Bet) == validActionsSb.end())
        << "SB should NOT be able to bet when there's already a higher bet";

    // Set up strategies to complete the hand properly
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerSb->getId(), pkt::core::ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    // BB wins when SB folds
    playerBb->setStrategy(std::move(bbStrategy));

    myHandFsm->runGameLoop();
}

TEST_F(BettingRoundsFsmTest, HeadsUpPositionAssignmentIsCorrect)
{
    logTestMessage("Testing that position assignment is correct in heads-up");

    initializeHandFsmWithPlayers(2, gameData);

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Setup simple scenario: SB folds preflop to keep it simple
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerSb->getId(), pkt::core::ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    // BB wins when SB folds - no action needed
    playerBb->setStrategy(std::move(bbStrategy));

    myHandFsm->runGameLoop();

    // Verify the position assignment is correct
    EXPECT_EQ(playerSb->getPosition(), PlayerPosition::ButtonSmallBlind)
        << "Player 0 should be Button+Small Blind in heads-up";
    EXPECT_EQ(playerBb->getPosition(), PlayerPosition::BigBlind) << "Player 1 should be Big Blind in heads-up";
}

TEST_F(BettingRoundsFsmTest, SmallBlindActsFirstPostflopInHeadsUp)
{
    logTestMessage("Testing that Small Blind acts first postflop in heads-up");

    initializeHandFsmWithPlayers(2, gameData);

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // In heads-up, SB is also button, so after SB calls, BB should have option to check
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerSb->getId(), pkt::core::ActionType::Call});
    sbStrategy->setLastAction(pkt::core::GameState::Flop, {playerSb->getId(), pkt::core::ActionType::Check});
    sbStrategy->setLastAction(pkt::core::GameState::Turn, {playerSb->getId(), pkt::core::ActionType::Check});
    sbStrategy->setLastAction(pkt::core::GameState::River, {playerSb->getId(), pkt::core::ActionType::Check});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    // Now that turn logic works correctly, BB needs to respond to SB's call
    bbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerBb->getId(), pkt::core::ActionType::Check});
    bbStrategy->setLastAction(pkt::core::GameState::Flop, {playerBb->getId(), pkt::core::ActionType::Check});
    bbStrategy->setLastAction(pkt::core::GameState::Turn, {playerBb->getId(), pkt::core::ActionType::Check});
    bbStrategy->setLastAction(pkt::core::GameState::River, {playerBb->getId(), pkt::core::ActionType::Check});
    playerBb->setStrategy(std::move(bbStrategy));

    myHandFsm->runGameLoop();

    const auto& handHistory = myHandFsm->getHandActionHistory();
    // Verify we reached flop
    auto flopRound = std::find_if(handHistory.begin(), handHistory.end(),
                                  [](const auto& round) { return round.round == GameState::Flop; });

    if (flopRound != handHistory.end())
    {
        EXPECT_FALSE(flopRound->actions.empty()) << "Flop round should have actions";

        // Verify Big Blind acts first on flop in heads-up
        unsigned firstActorId = flopRound->actions.front().first;
        EXPECT_EQ(firstActorId, playerBb->getId()) << "Big Blind should act first postflop in heads-up";
        EXPECT_EQ(playerSb->getPosition(), PlayerPosition::ButtonSmallBlind)
            << "Small Blind should be ButtonSmallBlind in heads-up";
    }
    else
    {
        FAIL() << "Should have reached flop round but didn't find it in action history";
    }
}

TEST_F(BettingRoundsFsmTest, FirstToActPostflopIsSmallBlindInThreePlayers)
{
    logTestMessage("Testing that Small Blind acts first postflop with 3 players");

    initializeHandFsmWithPlayers(3, gameData);

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Setup strategies to reach flop with all players
    auto dealerStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    dealerStrategy->setLastAction(pkt::core::GameState::Preflop, {playerDealer->getId(), pkt::core::ActionType::Call});
    dealerStrategy->setLastAction(pkt::core::GameState::Flop, {playerDealer->getId(), pkt::core::ActionType::Fold});
    playerDealer->setStrategy(std::move(dealerStrategy));

    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerSb->getId(), pkt::core::ActionType::Call});
    sbStrategy->setLastAction(pkt::core::GameState::Flop, {playerSb->getId(), pkt::core::ActionType::Check});
    sbStrategy->setLastAction(pkt::core::GameState::Turn, {playerSb->getId(), pkt::core::ActionType::Check});
    sbStrategy->setLastAction(pkt::core::GameState::River, {playerSb->getId(), pkt::core::ActionType::Check});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    bbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerBb->getId(), pkt::core::ActionType::Check});
    bbStrategy->setLastAction(pkt::core::GameState::Flop, {playerBb->getId(), pkt::core::ActionType::Check});
    bbStrategy->setLastAction(pkt::core::GameState::Turn, {playerBb->getId(), pkt::core::ActionType::Check});
    bbStrategy->setLastAction(pkt::core::GameState::River, {playerBb->getId(), pkt::core::ActionType::Check});
    playerBb->setStrategy(std::move(bbStrategy));

    myHandFsm->runGameLoop();

    // Verify we reached flop
    const auto& handHistory = myHandFsm->getHandActionHistory();
    auto flopRound = std::find_if(handHistory.begin(), handHistory.end(),
                                  [](const auto& round) { return round.round == GameState::Flop; });

    ASSERT_NE(flopRound, handHistory.end()) << "Should have reached flop round";
    EXPECT_FALSE(flopRound->actions.empty()) << "Flop round should have actions";

    // Verify Small Blind acts first on flop
    unsigned firstActorId = flopRound->actions.front().first;
    EXPECT_EQ(firstActorId, playerSb->getId()) << "Small Blind should act first postflop with 3 players";
    EXPECT_EQ(playerSb->getPosition(), PlayerPosition::SmallBlind) << "Player should be Small Blind";
}

TEST_F(BettingRoundsFsmTest, NoPlayerStartsPostFlopRoundWithRaise)
{
    logTestMessage("Testing that engine rejects raise actions when starting post-flop rounds");

    initializeHandFsmWithPlayers(3, gameData);

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Setup strategies to reach flop
    auto dealerStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    dealerStrategy->setLastAction(GameState::Preflop, {playerDealer->getId(), ActionType::Call});
    playerDealer->setStrategy(std::move(dealerStrategy));

    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Call});
    // Small blind will attempt an INVALID raise as first action on flop
    sbStrategy->setLastAction(GameState::Flop, {playerSb->getId(), ActionType::Raise, 50});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    bbStrategy->setLastAction(GameState::Preflop, {playerBb->getId(), ActionType::Check});
    playerBb->setStrategy(std::move(bbStrategy));

    // Track invalid actions from engine
    std::vector<std::tuple<unsigned, PlayerAction, std::string>> invalidActions;
    myEvents.onInvalidPlayerAction = [&invalidActions](unsigned playerId, PlayerAction action, std::string reason)
    { invalidActions.push_back(std::make_tuple(playerId, action, reason)); };

    // Run the hand - this should trigger the invalid raise on flop
    myHandFsm->runGameLoop();

    // Verify the engine rejected the invalid raise action
    EXPECT_FALSE(invalidActions.empty()) << "Engine should reject raise action at start of post-flop round";

    if (!invalidActions.empty())
    {
        bool foundInvalidRaise = false;
        for (const auto& [playerId, rejectedAction, reason] : invalidActions)
        {
            if (rejectedAction.type == ActionType::Raise && playerId == playerSb->getId())
            {
                foundInvalidRaise = true;
                logTestMessage("Engine correctly rejected invalid raise with reason: " + reason);

                // The reason should indicate why raising is not allowed
                EXPECT_FALSE(reason.empty()) << "Engine should provide a reason for rejecting the raise";

                break;
            }
        }

        EXPECT_TRUE(foundInvalidRaise)
            << "Should have found rejection of invalid raise action from small blind on flop";
    }

    // Verify we should have reached at least flop (even if invalid action occurred)
    EXPECT_TRUE(myLastGameState == GameState::Flop || myLastGameState == GameState::Turn ||
                myLastGameState == GameState::River || myLastGameState == GameState::PostRiver)
        << "Should have progressed beyond preflop despite invalid action. Last state: "
        << gameStateToString(myLastGameState);
}

// - NoPlayerBetsAfterRaise
// - NoPlayerChecksAfterBetOrRaise
// - OnlyOneBetAllowedPerRoundUnlessRaised
// - FoldedPlayerDoesNotReappearInLaterRounds
// - NoBettingInPostRiverRound
// - AllInPlayerDoesNotActAgain
// - NoExtraActionsAfterFinalCall
// - HeadsUpEndsImmediatelyOnFold

} // namespace pkt::test
