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
TEST_F(BettingRoundsFsmTest, ShouldRecordAllActionsInHandHistoryChronologically)
{
    logTestMessage("Testing that all player actions are recorded chronologically in hand history");

    initializeHandFsmWithPlayers(3, gameData);

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Set up strategies - dealer folds, SB and BB don't need to act much
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

    // Now we can use the hand-level action history for true chronological verification
    const auto& handHistory = myHandFsm->getHandActionHistory();
    EXPECT_FALSE(handHistory.empty()) << "Hand action history should not be empty";

    // Verify each round has recorded actions
    for (const auto& roundHistory : handHistory)
    {
        SCOPED_TRACE("Round: " + gameStateToString(roundHistory.round));
        EXPECT_FALSE(roundHistory.actions.empty())
            << "No actions recorded for round " + gameStateToString(roundHistory.round);

        // Verify all actions in this round are valid
        for (const auto& [playerId, action] : roundHistory.actions)
        {
            EXPECT_GE(playerId, 0u) << "Player ID should be valid";
            EXPECT_NE(action, ActionType::None) << "Action type should not be None";
        }
    }

    // Verify we have at least preflop actions
    auto preflopRound = std::find_if(handHistory.begin(), handHistory.end(),
                                     [](const auto& round) { return round.round == GameState::Preflop; });
    EXPECT_NE(preflopRound, handHistory.end()) << "Should have preflop round recorded";

    if (preflopRound != handHistory.end())
    {
        // In preflop, we should see blind posts first, then player decisions
        const auto& preflopActions = preflopRound->actions;
        EXPECT_GE(preflopActions.size(), 2u) << "Should have at least blind posts in preflop";

        // Verify chronological order: small blind post should come before big blind post
        bool foundSB = false, foundBB = false;
        size_t sbIndex = 0, bbIndex = 0;

        for (size_t i = 0; i < preflopActions.size(); ++i)
        {
            if (preflopActions[i].second == ActionType::PostSmallBlind)
            {
                foundSB = true;
                sbIndex = i;
            }
            else if (preflopActions[i].second == ActionType::PostBigBlind)
            {
                foundBB = true;
                bbIndex = i;
            }
        }

        if (foundSB && foundBB)
        {
            EXPECT_LT(sbIndex, bbIndex) << "Small blind should be posted before big blind in chronological order";
        }
    }

    // If we reached flop, verify flop actions are recorded after preflop
    auto flopRound = std::find_if(handHistory.begin(), handHistory.end(),
                                  [](const auto& round) { return round.round == GameState::Flop; });
    if (flopRound != handHistory.end())
    {
        // Flop round should come after preflop round in the history
        EXPECT_GT(std::distance(handHistory.begin(), flopRound), std::distance(handHistory.begin(), preflopRound))
            << "Flop round should come after preflop round in hand history";

        EXPECT_FALSE(flopRound->actions.empty()) << "Flop round should have actions recorded";
    }
}

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
