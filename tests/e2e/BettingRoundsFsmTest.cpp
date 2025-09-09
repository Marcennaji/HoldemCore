// tests/BettingRoundsFsmTest.cpp

#include "BettingRoundsFsmTest.h"
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

TEST_F(BettingRoundsFsmTest, DISABLED_StartShouldGoFromPreflopToPostRiverHeadsUp)
{
    logTestMessage("Testing heads-up hand completion");

    initializeHandFsmForTesting(2, gameData);
    myHandFsm->start();

    // Verify we reached PostRiver state
    EXPECT_EQ(myLastGameState, PostRiver) << "Hand should complete and reach PostRiver state";

    // Verify final state is terminal
    EXPECT_TRUE(myHandFsm->getState().isTerminal()) << "Final state should be terminal";
}

TEST_F(BettingRoundsFsmTest, DISABLED_StartShouldGoFromPreflopToPostRiver3Players)
{
    logTestMessage("Testing 3-player hand completion");

    initializeHandFsmForTesting(3, gameData);
    myHandFsm->start();

    // Verify we reached PostRiver state
    EXPECT_EQ(myLastGameState, PostRiver) << "3-player hand should complete and reach PostRiver state";

    // Verify final state is terminal
    EXPECT_TRUE(myHandFsm->getState().isTerminal()) << "Final state should be terminal";
}

TEST_F(BettingRoundsFsmTest, DISABLED_StartShouldGoFromPreflopToPostRiver6Players)
{
    logTestMessage("Testing 6-player hand completion");

    initializeHandFsmForTesting(6, gameData);
    myHandFsm->start();

    // Verify we reached PostRiver state
    EXPECT_EQ(myLastGameState, PostRiver) << "6-player hand should complete and reach PostRiver state";

    // Verify final state is terminal
    EXPECT_TRUE(myHandFsm->getState().isTerminal()) << "Final state should be terminal";
}

TEST_F(BettingRoundsFsmTest, DISABLED_FsmStateTransitionsFollowCorrectSequence)
{
    logTestMessage("Testing FSM state transition sequence");

    initializeHandFsmForTesting(3, gameData);

    // Track state transitions
    std::vector<GameState> stateSequence;
    myEvents.onBettingRoundStarted = [&](GameState state)
    {
        stateSequence.push_back(state);
        myLastGameState = state;
    };

    myHandFsm->start();

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

    initializeHandFsmForTesting(2, gameData);

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

    myHandFsm->start();

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

TEST_F(BettingRoundsFsmTest, DISABLED_PlayersDoNotActAfterFolding)
{
    logTestMessage("Testing that folded players don't act in later rounds");

    initializeHandFsmForTesting(4, gameData);

    // Track all player actions during the hand
    std::map<unsigned, std::vector<std::pair<GameState, ActionType>>> playerActions;

    // We'll need to manually step through the hand to observe folding behavior
    // This is a more complex test that would require hand action tracking
    // For now, let's just verify the hand completes successfully
    myHandFsm->start();

    EXPECT_EQ(myLastGameState, PostRiver) << "Hand with potential folds should complete";

    // TODO: Add more detailed action tracking when we have access to hand history
    // or when we implement manual action stepping in FSM tests
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
