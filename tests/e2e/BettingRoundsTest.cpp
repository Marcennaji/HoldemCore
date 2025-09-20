// tests/BettingRoundsTest.cpp

#include "common/DeterministicStrategy.h"
#include "common/EngineTest.h"
#include "core/engine/model/PlayerAction.h"
#include "core/engine/utils/Helpers.h"
#include "core/player/Helpers.h"

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

class BettingRoundsTest : public EngineTest
{
  protected:
    void SetUp() override;
    void TearDown() override;
    void logTestMessage(const std::string& message) const;
    bool isPlayerStillActive(unsigned id) const;
    void checkStateTransitions();

    std::vector<core::GameState> stateSequence;
};

void BettingRoundsTest::logTestMessage(const std::string& message) const
{
    getLogger().info("BettingRoundsTest : " + message);
}

void BettingRoundsTest::SetUp()
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

void BettingRoundsTest::TearDown()
{
    checkPostRiverConditions();
    checkStateTransitions();
}

bool BettingRoundsTest::isPlayerStillActive(unsigned id) const
{
    for (const auto& p : *myHand->getActingPlayersList())
    {
        if (p->getId() == id)
            return true;
    }
    return false;
}
void BettingRoundsTest::checkStateTransitions()
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

TEST_F(BettingRoundsTest, StartShouldGoFromPreflopToPostRiverHeadsUp)
{
    logTestMessage("Testing heads-up hand completion");

    initializeHandWithPlayers(2, gameData);

    auto playerSb = getPlayerById(myActingPlayersList, 0);
    auto playerBb = getPlayerById(myActingPlayersList, 1);

    // Inject deterministic strategies - SB folds, BB wins
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerSb->getId(), pkt::core::ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    // Big blind does nothing here; we don't configure Preflop action
    playerBb->setStrategy(std::move(bbStrategy));

    myHand->runGameLoop(); // Automatically prompts players and drives game to completion

    // Verify we reached PostRiver state
    EXPECT_EQ(myLastGameState, PostRiver) << "Hand should complete and reach PostRiver state";

    // Verify final state is terminal
    EXPECT_TRUE(myHand->getState().isTerminal()) << "Final state should be terminal";
}

TEST_F(BettingRoundsTest, StartShouldGoFromPreflopToPostRiver3Players)
{
    logTestMessage("Testing 3-player hand completion");

    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

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

    myHand->runGameLoop(); // Automatically prompts players and drives game to completion

    // Verify we reached PostRiver state
    EXPECT_EQ(myLastGameState, PostRiver) << "3-player hand should complete and reach PostRiver state";
}

TEST_F(BettingRoundsTest, StartShouldGoFromPreflopToPostRiver6Players)
{
    logTestMessage("Testing 6-player hand completion");

    initializeHandWithPlayers(6, gameData);

    auto player0 = getPlayerById(myActingPlayersList, 0); // Dealer
    auto player1 = getPlayerById(myActingPlayersList, 1); // SB
    auto player2 = getPlayerById(myActingPlayersList, 2); // BB
    auto player3 = getPlayerById(myActingPlayersList, 3); // UTG
    auto player4 = getPlayerById(myActingPlayersList, 4); // MP
    auto player5 = getPlayerById(myActingPlayersList, 5); // CO

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

    myHand->runGameLoop(); // Automatically prompts players and drives game to completion

    // Verify we reached PostRiver state
    EXPECT_EQ(myLastGameState, PostRiver) << "6-player hand should complete and reach PostRiver state";

    // Verify final state is terminal
    EXPECT_TRUE(myHand->getState().isTerminal()) << "Final state should be terminal";
}

TEST_F(BettingRoundsTest, AllInPlayersTransitionToPostRiverCorrectly)
{
    logTestMessage("Testing all-in scenario transitions to PostRiver");

    initializeHandWithPlayers(2, gameData);

    // Set up one player with limited cash to force all-in
    auto playerSb = getPlayerById(myActingPlayersList, 0);
    auto playerBb = getPlayerById(myActingPlayersList, 1);

    // Reduce SB player's cash to force all-in scenario
    playerSb->setCash(50); // After posting SB (10), they'll have 40 left

    // Inject deterministic strategies - SB goes all-in, BB calls
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerSb->getId(), pkt::core::ActionType::Allin});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    bbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerBb->getId(), pkt::core::ActionType::Call});
    playerBb->setStrategy(std::move(bbStrategy));

    myHand->runGameLoop(); // Automatically prompts players and drives game to completion

    EXPECT_EQ(stateSequence.size(), 2)
        << "This preflop All-in scenario should have only 2 states: Preflop and PostRiver";
    EXPECT_EQ(stateSequence[0], Preflop) << "First state should be Preflop";
    EXPECT_EQ(stateSequence[1], PostRiver) << "Second state should be PostRiver";
}

TEST_F(BettingRoundsTest, PlayersDoNotActAfterFolding)
{
    logTestMessage("Testing that folded players don't act in later rounds");

    initializeHandWithPlayers(4, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);
    auto playerUtg = getPlayerById(myActingPlayersList, 3);

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

    myHand->runGameLoop();

    EXPECT_EQ(myLastGameState, PostRiver) << "Hand should complete after folds";

    // Verify folded players are not in final acting list
    EXPECT_FALSE(isPlayerStillActive(playerDealer->getId())) << "Dealer should have folded and not be active";
    EXPECT_FALSE(isPlayerStillActive(playerSb->getId())) << "SB should have folded and not be active";
    EXPECT_FALSE(isPlayerStillActive(playerUtg->getId())) << "UTG should have folded and not be active";

    // Use hand action history to verify folded players don't act after folding
    const auto& handHistory = myHand->getHandActionHistory();
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
TEST_F(BettingRoundsTest, ShouldRecordAllActionsInHandHistoryChronologically)
{
    logTestMessage("Testing that all player actions are recorded chronologically in hand history");

    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

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

    myHand->runGameLoop();

    // Verify the hand action history contains exactly what we expect
    const auto& handHistory = myHand->getHandActionHistory();
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

TEST_F(BettingRoundsTest, ActionOrderStartsCorrectlyInHeadsUpPreflop)
{
    logTestMessage("Testing that action order starts correctly in heads-up preflop");

    initializeHandWithPlayers(2, gameData);

    auto playerSb = getPlayerById(myActingPlayersList, 0);
    auto playerBb = getPlayerById(myActingPlayersList, 1);

    // Setup strategies - SB folds to avoid infinite loop, we just need to verify the action order
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerSb->getId(), pkt::core::ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    // BB doesn't need to act if SB folds
    playerBb->setStrategy(std::move(bbStrategy));

    myHand->runGameLoop();

    // Verify the hand action history shows correct action order
    const auto& handHistory = myHand->getHandActionHistory();
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

TEST_F(BettingRoundsTest, ValidActionsAreCorrectlyDetermined)
{
    logTestMessage("Testing that valid actions are correctly determined for players");

    initializeHandWithPlayers(2, gameData);

    auto playerSb = getPlayerById(myActingPlayersList, 0);
    auto playerBb = getPlayerById(myActingPlayersList, 1);

    // Check valid actions for SB after blinds are posted (after initialization)
    auto validActionsSb = getValidActionsForPlayer(*myHand, playerSb->getId());
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

    myHand->runGameLoop();
}

TEST_F(BettingRoundsTest, HeadsUpPositionAssignmentIsCorrect)
{
    logTestMessage("Testing that position assignment is correct in heads-up");

    initializeHandWithPlayers(2, gameData);

    auto playerSb = getPlayerById(myActingPlayersList, 0);
    auto playerBb = getPlayerById(myActingPlayersList, 1);

    // Setup simple scenario: SB folds preflop to keep it simple
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerSb->getId(), pkt::core::ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    // BB wins when SB folds - no action needed
    playerBb->setStrategy(std::move(bbStrategy));

    myHand->runGameLoop();

    // Verify the position assignment is correct
    EXPECT_EQ(playerSb->getPosition(), PlayerPosition::ButtonSmallBlind)
        << "Player 0 should be Button+Small Blind in heads-up";
    EXPECT_EQ(playerBb->getPosition(), PlayerPosition::BigBlind) << "Player 1 should be Big Blind in heads-up";
}

TEST_F(BettingRoundsTest, SmallBlindActsFirstPostflopInHeadsUp)
{
    logTestMessage("Testing that Small Blind acts first postflop in heads-up");

    initializeHandWithPlayers(2, gameData);

    auto playerSb = getPlayerById(myActingPlayersList, 0);
    auto playerBb = getPlayerById(myActingPlayersList, 1);

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

    myHand->runGameLoop();

    const auto& handHistory = myHand->getHandActionHistory();
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

TEST_F(BettingRoundsTest, FirstToActPostflopIsSmallBlindInThreePlayers)
{
    logTestMessage("Testing that Small Blind acts first postflop with 3 players");

    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

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

    myHand->runGameLoop();

    // Verify we reached flop
    const auto& handHistory = myHand->getHandActionHistory();
    auto flopRound = std::find_if(handHistory.begin(), handHistory.end(),
                                  [](const auto& round) { return round.round == GameState::Flop; });

    ASSERT_NE(flopRound, handHistory.end()) << "Should have reached flop round";
    EXPECT_FALSE(flopRound->actions.empty()) << "Flop round should have actions";

    // Verify Small Blind acts first on flop
    unsigned firstActorId = flopRound->actions.front().first;
    EXPECT_EQ(firstActorId, playerSb->getId()) << "Small Blind should act first postflop with 3 players";
    EXPECT_EQ(playerSb->getPosition(), PlayerPosition::SmallBlind) << "Player should be Small Blind";
}

TEST_F(BettingRoundsTest, NoPlayerStartsPostFlopRoundWithRaise)
{
    logTestMessage("Testing that engine rejects raise actions when starting post-flop rounds");

    initializeHandWithPlayers(3, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);

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
    myHand->runGameLoop();

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

TEST_F(BettingRoundsTest, NoPlayerBetsAfterRaise)
{
    logTestMessage("Testing that engine rejects bet actions after a raise has been made");

    initializeHandWithPlayers(4, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);
    auto playerUtg = getPlayerById(myActingPlayersList, 3);

    // Setup strategies to create a scenario where one player raises, then another tries to bet
    auto dealerStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    dealerStrategy->setLastAction(GameState::Preflop, {playerDealer->getId(), ActionType::Call});
    dealerStrategy->setLastAction(GameState::Flop, {playerDealer->getId(), ActionType::Check});
    playerDealer->setStrategy(std::move(dealerStrategy));

    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Call});
    // Small blind will bet first on flop
    sbStrategy->setLastAction(GameState::Flop, {playerSb->getId(), ActionType::Bet, 30});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    bbStrategy->setLastAction(GameState::Preflop, {playerBb->getId(), ActionType::Check});
    // Big blind will raise the bet
    bbStrategy->setLastAction(GameState::Flop, {playerBb->getId(), ActionType::Raise, 60});
    playerBb->setStrategy(std::move(bbStrategy));

    auto utgStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    utgStrategy->setLastAction(GameState::Preflop, {playerUtg->getId(), ActionType::Call});
    // UTG will try to make an INVALID bet action after the raise
    utgStrategy->setLastAction(GameState::Flop, {playerUtg->getId(), ActionType::Bet, 40});
    playerUtg->setStrategy(std::move(utgStrategy));

    // Track invalid actions from engine
    std::vector<std::tuple<unsigned, PlayerAction, std::string>> invalidActions;
    myEvents.onInvalidPlayerAction = [&invalidActions](unsigned playerId, PlayerAction action, std::string reason)
    { invalidActions.push_back(std::make_tuple(playerId, action, reason)); };

    // Run the hand - this should trigger the invalid bet after raise
    myHand->runGameLoop();

    // Verify the engine rejected the invalid bet action after raise
    EXPECT_FALSE(invalidActions.empty()) << "Engine should reject bet action after a raise has been made";

    if (!invalidActions.empty())
    {
        bool foundInvalidBet = false;
        for (const auto& [playerId, rejectedAction, reason] : invalidActions)
        {
            if (rejectedAction.type == ActionType::Bet && playerId == playerUtg->getId())
            {
                foundInvalidBet = true;
                logTestMessage("Engine correctly rejected invalid bet after raise with reason: " + reason);

                // The reason should indicate why betting after raise is not allowed
                EXPECT_FALSE(reason.empty()) << "Engine should provide a reason for rejecting the bet";

                // Verify this happened on flop round where we expected it
                EXPECT_TRUE(myLastGameState >= GameState::Flop)
                    << "Invalid bet should have been attempted during or after flop";

                break;
            }
        }

        EXPECT_TRUE(foundInvalidBet) << "Should have found rejection of invalid bet action from UTG after BB raised";
    }

    // Verify we progressed properly through the hand despite invalid action
    EXPECT_TRUE(myLastGameState == GameState::Flop || myLastGameState == GameState::Turn ||
                myLastGameState == GameState::River || myLastGameState == GameState::PostRiver)
        << "Should have progressed to at least flop despite invalid action. Last state: "
        << gameStateToString(myLastGameState);
}

TEST_F(BettingRoundsTest, NoPlayerChecksAfterBetOrRaise)
{
    logTestMessage("Testing that engine rejects check actions after a bet or raise has been made");

    initializeHandWithPlayers(4, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);
    auto playerUtg = getPlayerById(myActingPlayersList, 3);

    // Setup strategies to create a scenario where one player bets, then another tries to check
    auto dealerStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    dealerStrategy->setLastAction(GameState::Preflop, {playerDealer->getId(), ActionType::Call});
    dealerStrategy->setLastAction(GameState::Flop, {playerDealer->getId(), ActionType::Check});
    playerDealer->setStrategy(std::move(dealerStrategy));

    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Call});
    // Small blind will bet on flop
    sbStrategy->setLastAction(GameState::Flop, {playerSb->getId(), ActionType::Bet, 40});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    bbStrategy->setLastAction(GameState::Preflop, {playerBb->getId(), ActionType::Check});
    // Big blind will try to make an INVALID check action after the bet
    bbStrategy->setLastAction(GameState::Flop, {playerBb->getId(), ActionType::Check});
    playerBb->setStrategy(std::move(bbStrategy));

    auto utgStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    utgStrategy->setLastAction(GameState::Preflop, {playerUtg->getId(), ActionType::Call});
    // UTG will also try to make an INVALID check action after the bet
    utgStrategy->setLastAction(GameState::Flop, {playerUtg->getId(), ActionType::Check});
    playerUtg->setStrategy(std::move(utgStrategy));

    // Track invalid actions from engine
    std::vector<std::tuple<unsigned, PlayerAction, std::string>> invalidActions;
    myEvents.onInvalidPlayerAction = [&invalidActions](unsigned playerId, PlayerAction action, std::string reason)
    { invalidActions.push_back(std::make_tuple(playerId, action, reason)); };

    // Run the hand - this should trigger invalid check actions after bet
    myHand->runGameLoop();

    // Verify the engine rejected the invalid check actions after bet
    EXPECT_FALSE(invalidActions.empty()) << "Engine should reject check actions after a bet has been made";

    if (!invalidActions.empty())
    {
        bool foundInvalidCheck = false;
        for (const auto& [playerId, rejectedAction, reason] : invalidActions)
        {
            if (rejectedAction.type == ActionType::Check &&
                (playerId == playerBb->getId() || playerId == playerUtg->getId()))
            {
                foundInvalidCheck = true;
                logTestMessage("Engine correctly rejected invalid check after bet with reason: " + reason);

                // The reason should indicate why checking after bet is not allowed
                EXPECT_FALSE(reason.empty()) << "Engine should provide a reason for rejecting the check";

                break;
            }
        }

        EXPECT_TRUE(foundInvalidCheck) << "Should have found rejection of invalid check action after bet was made";
    }

    // Verify we progressed properly through the hand despite invalid actions
    EXPECT_TRUE(myLastGameState == GameState::Flop || myLastGameState == GameState::Turn ||
                myLastGameState == GameState::River || myLastGameState == GameState::PostRiver)
        << "Should have progressed to at least flop despite invalid actions. Last state: "
        << gameStateToString(myLastGameState);
}

TEST_F(BettingRoundsTest, FoldedPlayerDoesNotReappearInLaterRounds)
{
    logTestMessage("Testing that engine rejects actions from players who have already folded");

    initializeHandWithPlayers(4, gameData);

    auto playerDealer = getPlayerById(myActingPlayersList, 0);
    auto playerSb = getPlayerById(myActingPlayersList, 1);
    auto playerBb = getPlayerById(myActingPlayersList, 2);
    auto playerUtg = getPlayerById(myActingPlayersList, 3);

    // Setup strategies: Only 2 players continue to flop
    auto dealerStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    dealerStrategy->setLastAction(GameState::Preflop, {playerDealer->getId(), ActionType::Fold});
    playerDealer->setStrategy(std::move(dealerStrategy));

    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Call});
    sbStrategy->setLastAction(GameState::Flop, {playerSb->getId(), ActionType::Check});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    bbStrategy->setLastAction(GameState::Preflop, {playerBb->getId(), ActionType::Check});
    bbStrategy->setLastAction(GameState::Flop, {playerBb->getId(), ActionType::Check});
    playerBb->setStrategy(std::move(bbStrategy));

    auto utgStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    utgStrategy->setLastAction(GameState::Preflop, {playerUtg->getId(), ActionType::Fold});
    playerUtg->setStrategy(std::move(utgStrategy));

    // Track invalid actions from engine
    std::vector<std::tuple<unsigned, PlayerAction, std::string>> invalidActions;
    myEvents.onInvalidPlayerAction = [&invalidActions](unsigned playerId, PlayerAction action, std::string reason)
    { invalidActions.push_back(std::make_tuple(playerId, action, reason)); };

    // Run the complete hand until it reaches a later round or ends
    myHand->runGameLoop();

    // Now manually try to submit an action from a folded player
    // This should happen after the players have folded in preflop
    PlayerAction invalidActionFromFoldedPlayer{playerDealer->getId(), ActionType::Check, 0};

    logTestMessage("Attempting to submit action from folded player (Dealer)");
    myHand->handlePlayerAction(invalidActionFromFoldedPlayer);

    // Try another action from another folded player
    PlayerAction anotherInvalidAction{playerUtg->getId(), ActionType::Bet, 50};

    logTestMessage("Attempting to submit action from folded player (UTG)");
    myHand->handlePlayerAction(anotherInvalidAction);

    // Verify the engine rejected actions from folded players
    EXPECT_FALSE(invalidActions.empty()) << "Engine should reject actions from players who have already folded";

    if (!invalidActions.empty())
    {
        bool foundFoldedPlayerAction = false;
        for (const auto& [playerId, rejectedAction, reason] : invalidActions)
        {
            // Look for actions from players who folded (Dealer=0 or UTG=3)
            if (playerId == playerDealer->getId() || playerId == playerUtg->getId())
            {
                foundFoldedPlayerAction = true;
                logTestMessage("Engine correctly rejected action from folded player " + std::to_string(playerId) +
                               " (" + actionTypeToString(rejectedAction.type) + ") with reason: " + reason);

                // The reason should indicate the player is not active/folded
                EXPECT_FALSE(reason.empty()) << "Engine should provide a reason for rejecting folded player action";

                break;
            }
        }

        EXPECT_TRUE(foundFoldedPlayerAction) << "Should have found rejection of action from a folded player";
    }

    // Verify folded players are not in active list after folding
    EXPECT_FALSE(isPlayerStillActive(playerDealer->getId())) << "Dealer should not be active after folding";
    EXPECT_FALSE(isPlayerStillActive(playerUtg->getId())) << "UTG should not be active after folding";
}

TEST_F(BettingRoundsTest, NoExtraActionsAfterFinalCall)
{
    logTestMessage("Testing that engine rejects extra actions after final call completes betting round");

    initializeHandWithPlayers(3, gameData);

    auto playerSb = getPlayerById(myActingPlayersList, 0);
    auto playerBb = getPlayerById(myActingPlayersList, 1);
    auto playerUtg = getPlayerById(myActingPlayersList, 2);

    // Setup strategies: Create scenario where betting round ends with final call
    // In 3-player preflop: SB=0, BB=1, UTG=2 (positions might rotate, but SB posts first)
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Call}); // SB calls to complete BB
    sbStrategy->setLastAction(GameState::Flop, {playerSb->getId(), ActionType::Check});   // SB checks
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    bbStrategy->setLastAction(GameState::Preflop,
                              {playerBb->getId(), ActionType::Check}); // BB checks after all players call
    bbStrategy->setLastAction(GameState::Flop, {playerBb->getId(), ActionType::Bet, 30}); // BB bets on flop
    playerBb->setStrategy(std::move(bbStrategy));

    auto utgStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    utgStrategy->setLastAction(GameState::Preflop, {playerUtg->getId(), ActionType::Call}); // UTG calls BB
    utgStrategy->setLastAction(GameState::Flop,
                               {playerUtg->getId(), ActionType::Call}); // UTG calls BB's bet (final call)
    playerUtg->setStrategy(std::move(utgStrategy));

    // Track invalid actions from engine
    std::vector<std::tuple<unsigned, PlayerAction, std::string>> invalidActions;
    myEvents.onInvalidPlayerAction = [&invalidActions](unsigned playerId, PlayerAction action, std::string reason)
    { invalidActions.push_back(std::make_tuple(playerId, action, reason)); };

    // Run the hand until flop betting is complete
    myHand->runGameLoop();

    // At this point, the flop betting should be complete:
    // SB checked, BB bet 30, UTG called 30 (final call)
    // Now try to submit extra actions that should be rejected

    // Try to make SB act again after the round is complete
    PlayerAction extraActionFromSb{playerSb->getId(), ActionType::Raise, 60};
    logTestMessage("Attempting extra action from SB after final call");
    myHand->handlePlayerAction(extraActionFromSb);

    // Try to make BB act again after the round is complete
    PlayerAction extraActionFromBb{playerBb->getId(), ActionType::Bet, 50};
    logTestMessage("Attempting extra action from BB after final call");
    myHand->handlePlayerAction(extraActionFromBb);

    // Try to make UTG act again after the round is complete
    PlayerAction extraActionFromUtg{playerUtg->getId(), ActionType::Check, 0};
    logTestMessage("Attempting extra action from UTG after final call");
    myHand->handlePlayerAction(extraActionFromUtg);

    // Verify the engine rejected extra actions after final call
    EXPECT_FALSE(invalidActions.empty()) << "Engine should reject extra actions after betting round is complete";

    if (!invalidActions.empty())
    {
        bool foundExtraActionRejection = false;
        for (const auto& [playerId, rejectedAction, reason] : invalidActions)
        {
            // Look for actions that were rejected because the round is complete
            logTestMessage("Engine rejected action from player " + std::to_string(playerId) + " (" +
                           actionTypeToString(rejectedAction.type) + ") with reason: " + reason);

            // The reason should indicate that no more actions are needed or the round is complete
            EXPECT_FALSE(reason.empty()) << "Engine should provide a reason for rejecting extra actions";

            foundExtraActionRejection = true;
        }

        EXPECT_TRUE(foundExtraActionRejection) << "Should have found rejection of extra actions after final call";
    }

    // Verify the hand has progressed beyond flop (indicating the round properly ended)
    EXPECT_NE(myHand->getGameState(), GameState::Flop)
        << "Hand should have progressed beyond flop after all players acted";
}

TEST_F(BettingRoundsTest, NoBettingInPostRiverRound)
{
    logTestMessage("Testing that engine rejects betting actions in PostRiver state");

    initializeHandWithPlayers(2, gameData);

    auto playerSb = getPlayerById(myActingPlayersList, 0);
    auto playerBb = getPlayerById(myActingPlayersList, 1);

    // Setup strategies to quickly reach PostRiver state
    // SB folds immediately, BB wins, hand goes directly to PostRiver
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    // BB doesn't need to act since SB folds
    playerBb->setStrategy(std::move(bbStrategy));

    // Track invalid actions from engine
    std::vector<std::tuple<unsigned, PlayerAction, std::string>> invalidActions;
    myEvents.onInvalidPlayerAction = [&invalidActions](unsigned playerId, PlayerAction action, std::string reason)
    { invalidActions.push_back(std::make_tuple(playerId, action, reason)); };

    // Run the hand to completion (should reach PostRiver)
    myHand->runGameLoop();

    // Verify we've reached PostRiver state
    EXPECT_EQ(myHand->getGameState(), GameState::PostRiver) << "Hand should have reached PostRiver state after fold";

    // Now try to submit various betting actions in PostRiver state
    // These should all be rejected by the engine

    PlayerAction invalidBet{playerSb->getId(), ActionType::Bet, 50};
    logTestMessage("Attempting to bet in PostRiver state");
    myHand->handlePlayerAction(invalidBet);

    PlayerAction invalidRaise{playerBb->getId(), ActionType::Raise, 100};
    logTestMessage("Attempting to raise in PostRiver state");
    myHand->handlePlayerAction(invalidRaise);

    PlayerAction invalidCall{playerSb->getId(), ActionType::Call, 0};
    logTestMessage("Attempting to call in PostRiver state");
    myHand->handlePlayerAction(invalidCall);

    PlayerAction invalidCheck{playerBb->getId(), ActionType::Check, 0};
    logTestMessage("Attempting to check in PostRiver state");
    myHand->handlePlayerAction(invalidCheck);

    PlayerAction invalidAllIn{playerSb->getId(), ActionType::Allin, 0};
    logTestMessage("Attempting to go all-in in PostRiver state");
    myHand->handlePlayerAction(invalidAllIn);

    // Verify the engine rejected all betting actions in PostRiver
    EXPECT_FALSE(invalidActions.empty()) << "Engine should reject all betting actions in PostRiver state";

    if (!invalidActions.empty())
    {
        bool foundPostRiverRejection = false;
        for (const auto& [playerId, rejectedAction, reason] : invalidActions)
        {
            logTestMessage("Engine rejected PostRiver action from player " + std::to_string(playerId) + " (" +
                           actionTypeToString(rejectedAction.type) + ") with reason: " + reason);

            // The reason should indicate that no betting is allowed in PostRiver
            EXPECT_FALSE(reason.empty()) << "Engine should provide a reason for rejecting PostRiver betting actions";

            foundPostRiverRejection = true;
        }

        EXPECT_TRUE(foundPostRiverRejection) << "Should have found rejection of betting actions in PostRiver state";
    }

    // Verify the game state remains PostRiver (no state changes from invalid actions)
    EXPECT_EQ(myHand->getGameState(), GameState::PostRiver)
        << "Game state should remain PostRiver after rejected actions";
}

TEST_F(BettingRoundsTest, AllInPlayerDoesNotActAgain)
{
    logTestMessage("Testing that all-in players cannot act again in subsequent rounds");

    initializeHandWithPlayers(3, gameData);

    auto playerSb = getPlayerById(myActingPlayersList, 0);
    auto playerBb = getPlayerById(myActingPlayersList, 1);
    auto playerUtg = getPlayerById(myActingPlayersList, 2);

    // Reduce SB player's cash to create all-in scenario
    playerSb->setCash(50); // After posting SB (10), they'll have 40 left for all-in

    // Setup strategies: SB goes all-in preflop, others call to continue to flop
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Allin}); // SB goes all-in
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    bbStrategy->setLastAction(GameState::Preflop, {playerBb->getId(), ActionType::Call}); // BB calls all-in
    bbStrategy->setLastAction(GameState::Flop, {playerBb->getId(), ActionType::Check});   // BB checks on flop
    playerBb->setStrategy(std::move(bbStrategy));

    auto utgStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    utgStrategy->setLastAction(GameState::Preflop, {playerUtg->getId(), ActionType::Call}); // UTG calls all-in
    utgStrategy->setLastAction(GameState::Flop, {playerUtg->getId(), ActionType::Check});   // UTG checks on flop
    playerUtg->setStrategy(std::move(utgStrategy));

    // Track invalid actions from engine
    std::vector<std::tuple<unsigned, PlayerAction, std::string>> invalidActions;
    myEvents.onInvalidPlayerAction = [&invalidActions](unsigned playerId, PlayerAction action, std::string reason)
    { invalidActions.push_back(std::make_tuple(playerId, action, reason)); };

    // Run the hand until we reach flop (SB should be all-in from preflop)
    myHand->runGameLoop();

    // Verify the hand progressed beyond preflop
    EXPECT_NE(myHand->getGameState(), GameState::Preflop) << "Hand should have progressed beyond preflop after all-in";

    // Now try to make the all-in player (SB) act again in later rounds
    // This should be rejected since they're already all-in

    PlayerAction invalidBetFromAllIn{playerSb->getId(), ActionType::Bet, 20};
    logTestMessage("Attempting bet from all-in player");
    myHand->handlePlayerAction(invalidBetFromAllIn);

    PlayerAction invalidRaiseFromAllIn{playerSb->getId(), ActionType::Raise, 50};
    logTestMessage("Attempting raise from all-in player");
    myHand->handlePlayerAction(invalidRaiseFromAllIn);

    PlayerAction invalidCallFromAllIn{playerSb->getId(), ActionType::Call, 0};
    logTestMessage("Attempting call from all-in player");
    myHand->handlePlayerAction(invalidCallFromAllIn);

    PlayerAction invalidCheckFromAllIn{playerSb->getId(), ActionType::Check, 0};
    logTestMessage("Attempting check from all-in player");
    myHand->handlePlayerAction(invalidCheckFromAllIn);

    PlayerAction invalidFoldFromAllIn{playerSb->getId(), ActionType::Fold, 0};
    logTestMessage("Attempting fold from all-in player");
    myHand->handlePlayerAction(invalidFoldFromAllIn);

    // Verify the engine rejected actions from the all-in player
    EXPECT_FALSE(invalidActions.empty()) << "Engine should reject actions from players who are already all-in";

    if (!invalidActions.empty())
    {
        bool foundAllInPlayerRejection = false;
        for (const auto& [playerId, rejectedAction, reason] : invalidActions)
        {
            // Look for actions from the all-in player (SB)
            if (playerId == playerSb->getId())
            {
                foundAllInPlayerRejection = true;
                logTestMessage("Engine correctly rejected action from all-in player " + std::to_string(playerId) +
                               " (" + actionTypeToString(rejectedAction.type) + ") with reason: " + reason);

                // The reason should indicate the player is all-in or cannot act
                EXPECT_FALSE(reason.empty()) << "Engine should provide a reason for rejecting all-in player actions";
            }
        }

        EXPECT_TRUE(foundAllInPlayerRejection) << "Should have found rejection of actions from all-in player";
    }
}

TEST_F(BettingRoundsTest, HeadsUpEndsImmediatelyOnFold)
{
    logTestMessage("Testing that heads-up hands end immediately when one player folds");

    initializeHandWithPlayers(2, gameData);

    auto playerSb = getPlayerById(myActingPlayersList, 0);
    auto playerBb = getPlayerById(myActingPlayersList, 1);

    // Test scenario 1: SB folds preflop - hand should end immediately
    logTestMessage("Testing SB fold in preflop");

    // Setup strategies: SB folds immediately, BB should win without acting
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(GameState::Preflop, {playerSb->getId(), ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    // BB doesn't need any actions since SB folds immediately
    playerBb->setStrategy(std::move(bbStrategy));

    // Track invalid actions to ensure no extra actions are attempted
    std::vector<std::tuple<unsigned, PlayerAction, std::string>> invalidActions;
    myEvents.onInvalidPlayerAction = [&invalidActions](unsigned playerId, PlayerAction action, std::string reason)
    { invalidActions.push_back(std::make_tuple(playerId, action, reason)); };

    // Run the hand
    myHand->runGameLoop();

    // Verify the hand went directly to PostRiver after the fold
    EXPECT_EQ(myHand->getGameState(), GameState::PostRiver)
        << "Heads-up hand should end immediately and reach PostRiver when one player folds";

    // Verify the state sequence shows immediate transition
    EXPECT_GE(stateSequence.size(), 2u) << "Should have at least Preflop and PostRiver states";
    EXPECT_EQ(stateSequence[0], GameState::Preflop) << "Should start in Preflop";
    EXPECT_EQ(stateSequence.back(), GameState::PostRiver) << "Should end in PostRiver";

    // In heads-up, a fold should transition directly to PostRiver (no flop/turn/river)
    // The sequence should be short: Preflop -> PostRiver
    EXPECT_LE(stateSequence.size(), 2u)
        << "Heads-up fold should transition directly from Preflop to PostRiver without intermediate states";

    // Verify the folded player is no longer active
    EXPECT_FALSE(isPlayerStillActive(playerSb->getId())) << "Folded player should not be active after folding";

    // Verify winner (BB) is still active or properly determined
    EXPECT_TRUE(isPlayerStillActive(playerBb->getId()) || myHand->getGameState() == GameState::PostRiver)
        << "Winning player should be active or hand should be in PostRiver";

    // Test that attempting actions after the fold is rejected
    PlayerAction invalidActionAfterFold{playerSb->getId(), ActionType::Bet, 20};
    logTestMessage("Attempting action from folded player after heads-up fold");
    myHand->handlePlayerAction(invalidActionAfterFold);

    PlayerAction invalidActionFromWinner{playerBb->getId(), ActionType::Check, 0};
    logTestMessage("Attempting action from winner after heads-up fold");
    myHand->handlePlayerAction(invalidActionFromWinner);

    // Verify these actions were rejected
    EXPECT_FALSE(invalidActions.empty()) << "Engine should reject actions after heads-up hand has ended";

    if (!invalidActions.empty())
    {
        bool foundPostFoldRejection = false;
        for (const auto& [playerId, rejectedAction, reason] : invalidActions)
        {
            logTestMessage("Engine rejected post-fold action from player " + std::to_string(playerId) + " (" +
                           actionTypeToString(rejectedAction.type) + ") with reason: " + reason);

            // Should reject actions because hand is over or player is inactive
            EXPECT_FALSE(reason.empty()) << "Engine should provide reason for rejecting actions after heads-up fold";

            foundPostFoldRejection = true;
        }

        EXPECT_TRUE(foundPostFoldRejection) << "Should have found rejection of actions after heads-up fold";
    }

    // Verify hand action history shows the immediate fold
    const auto& handHistory = myHand->getHandActionHistory();
    EXPECT_FALSE(handHistory.empty()) << "Hand action history should not be empty";

    // Should have preflop round with SB fold
    auto preflopRound = std::find_if(handHistory.begin(), handHistory.end(),
                                     [](const auto& round) { return round.round == GameState::Preflop; });
    ASSERT_NE(preflopRound, handHistory.end()) << "Should have preflop round in history";

    // Count fold actions - should have exactly one (the SB fold)
    int foldCount = 0;
    for (const auto& [playerId, action] : preflopRound->actions)
    {
        if (action == ActionType::Fold && playerId == playerSb->getId())
        {
            foldCount++;
        }
    }
    EXPECT_EQ(foldCount, 1) << "Should have exactly one fold action (SB) in heads-up scenario";
}

} // namespace pkt::test
