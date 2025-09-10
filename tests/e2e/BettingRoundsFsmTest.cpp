// tests/BettingRoundsFsmTest.cpp

#include "BettingRoundsFsmTest.h"
#include "common/DeterministicStrategy.h"
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
    myEvents.onBettingRoundStarted = [&](GameState state) { myLastGameState = state; };
}

void BettingRoundsFsmTest::TearDown()
{
    checkPostRiverConditions();
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

TEST_F(BettingRoundsFsmTest, FsmStateTransitionsFollowCorrectSequence)
{
    logTestMessage("Testing FSM state transition sequence");

    initializeHandFsmWithPlayers(3, gameData);

    // Track state transitions
    std::vector<GameState> stateSequence;
    myEvents.onBettingRoundStarted = [&](GameState state)
    {
        stateSequence.push_back(state);
        myLastGameState = state;
    };

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Inject deterministic strategies - simple fold sequence to avoid infinite loops
    auto dealerStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    dealerStrategy->setLastAction(pkt::core::GameState::Preflop, {playerDealer->getId(), pkt::core::ActionType::Fold});
    playerDealer->setStrategy(std::move(dealerStrategy));

    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerSb->getId(), pkt::core::ActionType::Fold});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    // Big blind doesn't need to act in this scenario - wins by default
    playerBb->setStrategy(std::move(bbStrategy));

    myHandFsm->runGameLoop(); // Automatically prompts players and drives game to completion

    // Verify we have a valid sequence
    EXPECT_FALSE(stateSequence.empty()) << "Should have recorded state transitions";

    // Verify PostRiver is the final state
    EXPECT_EQ(stateSequence.back(), PostRiver) << "Final state should be PostRiver";

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

        EXPECT_TRUE(validTransition) << "Invalid transition from " << static_cast<int>(prev) << " to "
                                     << static_cast<int>(curr);
    }
}

TEST_F(BettingRoundsFsmTest, DISABLED_AllInPlayersTransitionToPostRiverCorrectly)
{
    logTestMessage("Testing all-in scenario transitions to PostRiver");

    initializeHandFsmWithPlayers(2, gameData);

    // Set up one player with limited cash to force all-in
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Reduce SB player's cash to force all-in scenario
    playerSb->setCash(50); // After posting SB (10), they'll have 40 left

    // Track state transitions
    std::vector<GameState> stateSequence;
    myEvents.onBettingRoundStarted = [&](GameState state)
    {
        stateSequence.push_back(state);
        myLastGameState = state;
    };

    // Inject deterministic strategies - SB goes all-in, BB calls
    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerSb->getId(), pkt::core::ActionType::Allin});
    playerSb->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    bbStrategy->setLastAction(pkt::core::GameState::Preflop, {playerBb->getId(), pkt::core::ActionType::Call});
    playerBb->setStrategy(std::move(bbStrategy));

    myHandFsm->runGameLoop(); // Automatically prompts players and drives game to completion

    // In an all-in scenario, we should transition directly to PostRiver
    // without going through all betting rounds
    EXPECT_EQ(myLastGameState, PostRiver) << "All-in scenario should end in PostRiver";

    // The state sequence should not include Turn and River if all-in happened early
    bool hasAllInTransition = false;
    for (size_t i = 1; i < stateSequence.size(); ++i)
    {
        if (stateSequence[i - 1] != PostRiver && stateSequence[i] == PostRiver)
        {
            hasAllInTransition = true;
            break;
        }
    }

    EXPECT_TRUE(hasAllInTransition) << "Should have direct transition to PostRiver in all-in scenario";
}

TEST_F(BettingRoundsFsmTest, PlayersDoNotActAfterFolding)
{
    logTestMessage("Testing that folded players don't act in later rounds");

    initializeHandFsmWithPlayers(4, gameData);

    // Track all player actions during the hand
    std::map<unsigned, std::vector<std::pair<GameState, ActionType>>> playerActions;

    auto player0 = getPlayerFsmById(myActingPlayersListFsm, 0); // Dealer
    auto player1 = getPlayerFsmById(myActingPlayersListFsm, 1); // SB
    auto player2 = getPlayerFsmById(myActingPlayersListFsm, 2); // BB
    auto player3 = getPlayerFsmById(myActingPlayersListFsm, 3); // UTG

    // Inject deterministic strategies - first two players fold early, last two never get to act
    auto dealerStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    dealerStrategy->setLastAction(pkt::core::GameState::Preflop, {player0->getId(), pkt::core::ActionType::Fold});
    player0->setStrategy(std::move(dealerStrategy));

    auto sbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    sbStrategy->setLastAction(pkt::core::GameState::Preflop, {player1->getId(), pkt::core::ActionType::Fold});
    player1->setStrategy(std::move(sbStrategy));

    auto bbStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    // BB wins by default when others fold
    player2->setStrategy(std::move(bbStrategy));

    auto utgStrategy = std::make_unique<pkt::test::DeterministicStrategy>();
    utgStrategy->setLastAction(pkt::core::GameState::Preflop, {player3->getId(), pkt::core::ActionType::Fold});
    player3->setStrategy(std::move(utgStrategy));

    myHandFsm->runGameLoop(); // Automatically prompts players and drives game to completion

    EXPECT_EQ(myLastGameState, PostRiver) << "Hand with potential folds should complete";

    // TODO: Add more detailed action tracking when we have access to hand history
    // or when we implement manual action stepping in FSM tests
    // For now, verify that the folded players (0 and 1) are not in the final acting list
    auto finalActingPlayers = myHandFsm->getActingPlayersList();
    EXPECT_FALSE(isPlayerStillActive(0)) << "Dealer should have folded and not be active";
    EXPECT_FALSE(isPlayerStillActive(1)) << "SB should have folded and not be active";
}

// TODO: Add more tests following the BettingRoundsLegacyTest pattern:
// - ShouldRecordAllActionsInHandHistoryChronologically (when FSM hand history is available)
// - ActionOrderStartsCorrectlyInHeadsUpPreflop
// - FirstToActPostflopIsLeftOfDealer
// - AllActionsAreFromActivePlayersOnly
// - NoTwoConsecutiveActionsBySamePlayerInRound
// - NoPlayerStartsPostFlopRoundWithRaise
// - NoPlayerBetsAfterRaise
// - NoPlayerChecksAfterBetOrRaise
// - OnlyOneBetAllowedPerRoundUnlessRaised
// - FoldedPlayerDoesNotReappearInLaterRounds
// - NoBettingInPostRiverRound
// - AllInPlayerDoesNotActAgain
// - NoExtraActionsAfterFinalCall
// - HeadsUpEndsImmediatelyOnFold

} // namespace pkt::test
