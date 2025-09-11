// tests/unit/engine/ActionsTest.cpp

#include "ActionsTest.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/Helpers.h"
#include "core/services/GlobalServices.h"

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

// ==================== ActionsPreflopTest Implementation ====================

void ActionsPreflopTest::logTestMessage(const std::string& message) const
{
    GlobalServices::instance().logger().verbose("ActionsPreflopTest : " + message);
}

void ActionsPreflopTest::SetUp()
{
    EngineTest::SetUp();
    myEvents.clear();
    myEvents.onBettingRoundStarted = [&](GameState state) { myLastGameState = state; };
}

void ActionsPreflopTest::TearDown()
{
    EngineTest::TearDown();
}

void ActionsPreflopTest::setupBasicHeadsUpScenario()
{
    initializeHandFsmWithPlayers(2, gameData);
}

void ActionsPreflopTest::setupThreePlayerScenario()
{
    initializeHandFsmWithPlayers(3, gameData);
}

void ActionsPreflopTest::setupPlayerWithLimitedCash(int playerId, int cash)
{
    auto player = getPlayerFsmById(myActingPlayersListFsm, playerId);
    ASSERT_NE(player, nullptr) << "Player with ID " << playerId << " not found";
    player->setCash(cash);
}

void ActionsPreflopTest::simulatePlayerAction(int playerId, ActionType actionType, int amount)
{
    PlayerAction action{playerId, actionType, amount};
    myHandFsm->handlePlayerAction(action);
}

bool ActionsPreflopTest::containsAction(const std::vector<ActionType>& actions, ActionType action)
{
    return std::find(actions.begin(), actions.end(), action) != actions.end();
}

bool ActionsPreflopTest::doesNotContainAction(const std::vector<ActionType>& actions, ActionType action)
{
    return std::find(actions.begin(), actions.end(), action) == actions.end();
}

// ==================== Tests for getValidActionsForPlayer ====================

TEST_F(ActionsPreflopTest, GetValidActionsForPlayerInitialPreflopSmallBlind)
{
    logTestMessage("Testing valid actions for small blind at start of preflop");

    setupBasicHeadsUpScenario();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto validActions = getValidActionsForPlayer(*myHandFsm, playerSb->getId());

    EXPECT_FALSE(validActions.empty()) << "Small blind should have valid actions";

    // SB should be able to fold, call, raise, or go all-in
    EXPECT_TRUE(containsAction(validActions, ActionType::Fold)) << "SB should be able to fold";
    EXPECT_TRUE(containsAction(validActions, ActionType::Call)) << "SB should be able to call";
    EXPECT_TRUE(containsAction(validActions, ActionType::Raise)) << "SB should be able to raise";
    EXPECT_TRUE(containsAction(validActions, ActionType::Allin)) << "SB should be able to go all-in";

    // SB should NOT be able to check (there's a bet to call) or bet (BB already posted bigger blind)
    EXPECT_TRUE(doesNotContainAction(validActions, ActionType::Check)) << "SB should NOT be able to check";
    EXPECT_TRUE(doesNotContainAction(validActions, ActionType::Bet)) << "SB should NOT be able to bet";
}

TEST_F(ActionsPreflopTest, GetValidActionsForPlayerAfterCall)
{
    logTestMessage("Testing valid actions for big blind after small blind calls");

    setupBasicHeadsUpScenario();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // SB calls
    simulatePlayerAction(playerSb->getId(), ActionType::Call);

    auto validActions = getValidActionsForPlayer(*myHandFsm, playerBb->getId());

    EXPECT_FALSE(validActions.empty()) << "Big blind should have valid actions after SB calls";

    // BB should be able to check (pots are equal), raise, or go all-in
    EXPECT_TRUE(containsAction(validActions, ActionType::Check)) << "BB should be able to check after call";
    EXPECT_TRUE(containsAction(validActions, ActionType::Raise)) << "BB should be able to raise";
    EXPECT_TRUE(containsAction(validActions, ActionType::Allin)) << "BB should be able to go all-in";
    EXPECT_TRUE(containsAction(validActions, ActionType::Fold)) << "BB should be able to fold";

    // BB should NOT be able to call (no additional bet) or bet (should use raise)
    EXPECT_TRUE(doesNotContainAction(validActions, ActionType::Call))
        << "BB should NOT be able to call when no additional bet";
    EXPECT_TRUE(doesNotContainAction(validActions, ActionType::Bet))
        << "BB should NOT be able to bet when pots are equal";
}

TEST_F(ActionsPreflopTest, GetValidActionsForPlayerWithLimitedCash)
{
    logTestMessage("Testing valid actions for player with very limited cash");

    setupBasicHeadsUpScenario();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);

    // Set SB to have very little cash (less than what's needed for minimum raise)
    setupPlayerWithLimitedCash(playerSb->getId(), 15); // Only 5 chips left after posting SB

    auto validActions = getValidActionsForPlayer(*myHandFsm, playerSb->getId());

    EXPECT_FALSE(validActions.empty()) << "Player with limited cash should still have some valid actions";

    // Player should be able to fold, call, or go all-in
    EXPECT_TRUE(containsAction(validActions, ActionType::Fold)) << "Player should be able to fold";
    EXPECT_TRUE(containsAction(validActions, ActionType::Call)) << "Player should be able to call";
    EXPECT_TRUE(containsAction(validActions, ActionType::Allin)) << "Player should be able to go all-in";

    // Player should NOT be able to raise (insufficient chips for minimum raise)
    EXPECT_TRUE(doesNotContainAction(validActions, ActionType::Raise))
        << "Player with limited cash should NOT be able to raise";
    EXPECT_TRUE(doesNotContainAction(validActions, ActionType::Check))
        << "Player should NOT be able to check when there's a bet";
    EXPECT_TRUE(doesNotContainAction(validActions, ActionType::Bet))
        << "Player should NOT be able to bet when there's a higher bet";
}

TEST_F(ActionsPreflopTest, GetValidActionsForPlayerAllIn)
{
    logTestMessage("Testing valid actions for player who is already all-in");

    setupBasicHeadsUpScenario();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);

    // Set player to have no cash (simulating all-in state)
    setupPlayerWithLimitedCash(playerSb->getId(), 10); // Exactly the SB amount, so 0 left after posting

    // Debug: Check player's actual cash after posting blind
    logTestMessage("Player SB cash after setup: " + std::to_string(playerSb->getCash()));

    auto validActions = getValidActionsForPlayer(*myHandFsm, playerSb->getId());

    // Player with no cash can still fold, but cannot make any other action that requires chips
    EXPECT_FALSE(validActions.empty()) << "Player should still be able to fold even with no cash";
    EXPECT_TRUE(containsAction(validActions, ActionType::Fold)) << "Player with no cash should be able to fold";

    // If player truly has 0 cash, they should not be able to make moves requiring chips
    if (playerSb->getCash() == 0)
    {
        EXPECT_TRUE(doesNotContainAction(validActions, ActionType::Call))
            << "Player with no cash should NOT be able to call";
        EXPECT_TRUE(doesNotContainAction(validActions, ActionType::Raise))
            << "Player with no cash should NOT be able to raise";
        EXPECT_TRUE(doesNotContainAction(validActions, ActionType::Bet))
            << "Player with no cash should NOT be able to bet";
        EXPECT_TRUE(doesNotContainAction(validActions, ActionType::Allin))
            << "Player with no cash should NOT be able to go all-in again";
        EXPECT_TRUE(doesNotContainAction(validActions, ActionType::Check))
            << "Player with no cash should NOT be able to check when there's a bet";
    }
    else
    {
        // If player still has some cash, all-in should be available
        EXPECT_TRUE(containsAction(validActions, ActionType::Allin)) << "Player with cash should be able to go all-in";
    }
}

TEST_F(ActionsPreflopTest, GetValidActionsForPlayerInThreePlayerScenario)
{
    logTestMessage("Testing valid actions in three-player scenario");

    setupThreePlayerScenario();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // Test dealer's initial actions (first to act after blinds)
    auto validActionsDealer = getValidActionsForPlayer(*myHandFsm, playerDealer->getId());

    EXPECT_FALSE(validActionsDealer.empty()) << "Dealer should have valid actions";

    // Dealer should be able to fold, call BB, raise, or go all-in
    EXPECT_TRUE(containsAction(validActionsDealer, ActionType::Fold)) << "Dealer should be able to fold";
    EXPECT_TRUE(containsAction(validActionsDealer, ActionType::Call)) << "Dealer should be able to call BB";
    EXPECT_TRUE(containsAction(validActionsDealer, ActionType::Raise)) << "Dealer should be able to raise";
    EXPECT_TRUE(containsAction(validActionsDealer, ActionType::Allin)) << "Dealer should be able to go all-in";

    // Dealer should NOT be able to check or bet
    EXPECT_TRUE(doesNotContainAction(validActionsDealer, ActionType::Check)) << "Dealer should NOT be able to check";
    EXPECT_TRUE(doesNotContainAction(validActionsDealer, ActionType::Bet)) << "Dealer should NOT be able to bet";
}

TEST_F(ActionsPreflopTest, GetValidActionsForNonExistentPlayer)
{
    logTestMessage("Testing valid actions for non-existent player");

    setupBasicHeadsUpScenario();

    // Try to get valid actions for a player that doesn't exist
    auto validActions = getValidActionsForPlayer(*myHandFsm, 999);

    EXPECT_TRUE(validActions.empty()) << "Non-existent player should have no valid actions";
}

// ==================== Tests for validatePlayerAction ====================

TEST_F(ActionsPreflopTest, ValidatePlayerActionFoldAlwaysValid)
{
    logTestMessage("Testing that fold action is always valid for active players");

    setupBasicHeadsUpScenario();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    PlayerAction foldAction{playerSb->getId(), ActionType::Fold, 0};

    bool isValid = validatePlayerAction(myActingPlayersListFsm, foldAction, *myHandFsm->getBettingActions(),
                                        myHandFsm->getSmallBlind(), myHandFsm->getGameState());

    EXPECT_TRUE(isValid) << "Fold action should always be valid for active players";
}

TEST_F(ActionsPreflopTest, ValidatePlayerActionCallValid)
{
    logTestMessage("Testing that call action is valid when there's a bet to call");

    setupBasicHeadsUpScenario();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    PlayerAction callAction{playerSb->getId(), ActionType::Call, 10};

    bool isValid = validatePlayerAction(myActingPlayersListFsm, callAction, *myHandFsm->getBettingActions(),
                                        myHandFsm->getSmallBlind(), myHandFsm->getGameState());

    EXPECT_TRUE(isValid) << "Call action should be valid when there's a bet to call";
}

TEST_F(ActionsPreflopTest, ValidatePlayerActionCheckInvalidWhenBetToCall)
{
    logTestMessage("Testing that check action is invalid when there's a bet to call");

    setupBasicHeadsUpScenario();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    PlayerAction checkAction{playerSb->getId(), ActionType::Check, 0};

    bool isValid = validatePlayerAction(myActingPlayersListFsm, checkAction, *myHandFsm->getBettingActions(),
                                        myHandFsm->getSmallBlind(), myHandFsm->getGameState());

    EXPECT_FALSE(isValid) << "Check action should be invalid when there's a bet to call";
}

TEST_F(ActionsPreflopTest, ValidatePlayerActionRaiseValid)
{
    logTestMessage("Testing that raise action is valid with proper amount");

    setupBasicHeadsUpScenario();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);

    // Valid raise: double the big blind
    int raiseAmount = myHandFsm->getSmallBlind() * 4; // 40 when SB is 10, BB is 20
    PlayerAction raiseAction{playerSb->getId(), ActionType::Raise, raiseAmount};

    bool isValid = validatePlayerAction(myActingPlayersListFsm, raiseAction, *myHandFsm->getBettingActions(),
                                        myHandFsm->getSmallBlind(), myHandFsm->getGameState());

    EXPECT_TRUE(isValid) << "Raise action should be valid with proper amount";
}

TEST_F(ActionsPreflopTest, ValidatePlayerActionRaiseInvalidWithTooSmallAmount)
{
    logTestMessage("Testing that raise action is invalid with too small amount");

    setupBasicHeadsUpScenario();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);

    // Invalid raise: less than minimum raise
    int raiseAmount = myHandFsm->getSmallBlind() * 2; // 20 when BB is 20 - not enough
    PlayerAction raiseAction{playerSb->getId(), ActionType::Raise, raiseAmount};

    bool isValid = validatePlayerAction(myActingPlayersListFsm, raiseAction, *myHandFsm->getBettingActions(),
                                        myHandFsm->getSmallBlind(), myHandFsm->getGameState());

    EXPECT_FALSE(isValid) << "Raise action should be invalid with too small amount";
}

TEST_F(ActionsPreflopTest, ValidatePlayerActionBetInvalidWhenBetExists)
{
    logTestMessage("Testing that bet action is invalid when there's already a bet");

    setupBasicHeadsUpScenario();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    PlayerAction betAction{playerSb->getId(), ActionType::Bet, 30};

    bool isValid = validatePlayerAction(myActingPlayersListFsm, betAction, *myHandFsm->getBettingActions(),
                                        myHandFsm->getSmallBlind(), myHandFsm->getGameState());

    EXPECT_FALSE(isValid) << "Bet action should be invalid when there's already a bet (should use raise)";
}

TEST_F(ActionsPreflopTest, ValidatePlayerActionAllinValid)
{
    logTestMessage("Testing that all-in action is valid for players with chips");

    setupBasicHeadsUpScenario();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    PlayerAction allinAction{playerSb->getId(), ActionType::Allin, playerSb->getCash()};

    bool isValid = validatePlayerAction(myActingPlayersListFsm, allinAction, *myHandFsm->getBettingActions(),
                                        myHandFsm->getSmallBlind(), myHandFsm->getGameState());

    EXPECT_TRUE(isValid) << "All-in action should be valid for players with chips";
}

TEST_F(ActionsPreflopTest, ValidatePlayerActionInvalidPlayerId)
{
    logTestMessage("Testing validation with invalid player ID");

    setupBasicHeadsUpScenario();

    PlayerAction invalidAction{999, ActionType::Fold, 0};

    bool isValid = validatePlayerAction(myActingPlayersListFsm, invalidAction, *myHandFsm->getBettingActions(),
                                        myHandFsm->getSmallBlind(), myHandFsm->getGameState());

    EXPECT_FALSE(isValid) << "Action should be invalid for non-existent player";
}
TEST_F(ActionsPreflopTest, NoTwoConsecutiveActionsBySamePlayerInRound)
{
    logTestMessage("Testing that engine prevents consecutive actions by same player");

    setupThreePlayerScenario();

    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    // This should succeed (dealer acts first in 3-player scenario)
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Call});

    // this should be ignored as duplicate (dealer acting again consecutively)
    myHandFsm->handlePlayerAction({playerDealer->getId(), ActionType::Call});

    // this should succeed
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Call});

    // this should be ignored as duplicate (SB acting again consecutively)
    myHandFsm->handlePlayerAction({playerSb->getId(), ActionType::Raise, 50});

    EXPECT_EQ(myLastGameState, Preflop);

    const auto& history = myHandFsm->getHandActionHistory();

    // Verify no consecutive actions exist in the history
    int nbActions = 0;
    for (const auto& round : history)
    {
        unsigned lastPlayerId = std::numeric_limits<unsigned>::max();

        for (const auto& [playerId, action] : round.actions)
        {
            EXPECT_NE(playerId, lastPlayerId) << "No consecutive actions should exist in history - found player "
                                              << playerId << " acting after player " << lastPlayerId;
            lastPlayerId = playerId;
            nbActions++;
        }
    }
    EXPECT_EQ(4, nbActions) << "There should be only 4 valid actions in history (BB post, SB post, dealer call, SB "
                               "call), invalid actions ignored";
}

// ==================== Integration Tests ====================

TEST_F(ActionsPreflopTest, IntegrationValidActionsMatchValidation)
{
    logTestMessage("Testing that getValidActionsForPlayer results match validatePlayerAction");

    setupBasicHeadsUpScenario();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto validActions = getValidActionsForPlayer(*myHandFsm, playerSb->getId());

    EXPECT_FALSE(validActions.empty()) << "Player should have valid actions";

    // Test each valid action type
    for (ActionType actionType : validActions)
    {
        int amount = 0;
        if (actionType == ActionType::Raise)
        {
            amount = myHandFsm->getSmallBlind() * 4; // Valid raise amount
        }
        else if (actionType == ActionType::Bet)
        {
            amount = myHandFsm->getSmallBlind() * 2; // Valid bet amount
        }
        else if (actionType == ActionType::Call)
        {
            amount = 10; // Amount to call SB to BB
        }
        else if (actionType == ActionType::Allin)
        {
            amount = playerSb->getCash();
        }

        PlayerAction action{playerSb->getId(), actionType, amount};
        bool isValid = validatePlayerAction(myActingPlayersListFsm, action, *myHandFsm->getBettingActions(),
                                            myHandFsm->getSmallBlind(), myHandFsm->getGameState());

        EXPECT_TRUE(isValid) << "Action " << static_cast<int>(actionType)
                             << " should be valid according to validatePlayerAction";
    }
}

TEST_F(ActionsPreflopTest, IntegrationInvalidActionsAreRejected)
{
    logTestMessage("Testing that invalid actions are rejected by both functions");

    setupBasicHeadsUpScenario();

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto validActions = getValidActionsForPlayer(*myHandFsm, playerSb->getId());

    // Test that check is not in valid actions and fails validation
    if (doesNotContainAction(validActions, ActionType::Check))
    {
        PlayerAction checkAction{playerSb->getId(), ActionType::Check, 0};
        bool isValid = validatePlayerAction(myActingPlayersListFsm, checkAction, *myHandFsm->getBettingActions(),
                                            myHandFsm->getSmallBlind(), myHandFsm->getGameState());

        EXPECT_FALSE(isValid) << "Check action should be invalid according to validatePlayerAction";
    }

    // Test that bet is not in valid actions and fails validation
    if (doesNotContainAction(validActions, ActionType::Bet))
    {
        PlayerAction betAction{playerSb->getId(), ActionType::Bet, 30};
        bool isValid = validatePlayerAction(myActingPlayersListFsm, betAction, *myHandFsm->getBettingActions(),
                                            myHandFsm->getSmallBlind(), myHandFsm->getGameState());

        EXPECT_FALSE(isValid) << "Bet action should be invalid according to validatePlayerAction";
    }
}

// ==================== ActionsPostflopTest Implementation ====================

void ActionsPostflopTest::logTestMessage(const std::string& message) const
{
    GlobalServices::instance().logger().verbose("ActionsPostflopTest : " + message);
}

void ActionsPostflopTest::SetUp()
{
    EngineTest::SetUp();
    myEvents.clear();
    myEvents.onBettingRoundStarted = [&](GameState state) { myLastGameState = state; };
}

void ActionsPostflopTest::TearDown()
{
    EngineTest::TearDown();
}

void ActionsPostflopTest::setupBasicHeadsUpScenario()
{
    initializeHandFsmWithPlayers(2, gameData);
}

void ActionsPostflopTest::setupThreePlayerScenario()
{
    initializeHandFsmWithPlayers(3, gameData);
}

void ActionsPostflopTest::setupPlayerWithLimitedCash(int playerId, int cash)
{
    auto player = getPlayerFsmById(myActingPlayersListFsm, playerId);
    ASSERT_NE(player, nullptr) << "Player with ID " << playerId << " not found";
    player->setCash(cash);
}

void ActionsPostflopTest::simulatePlayerAction(int playerId, ActionType actionType, int amount)
{
    PlayerAction action{playerId, actionType, amount};
    myHandFsm->handlePlayerAction(action);
}

void ActionsPostflopTest::advanceToPostflop(GameState targetState)
{
    // Advance from preflop to the target state by simulating actions
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // Complete preflop betting (SB calls, BB checks)
    simulatePlayerAction(playerSb->getId(), ActionType::Call);
    simulatePlayerAction(playerBb->getId(), ActionType::Check);

    // Now we should be at Flop
    EXPECT_EQ(myHandFsm->getGameState(), GameState::Flop);

    if (targetState == GameState::Flop)
    {
        return; // We're done
    }

    // Advance to Turn if needed
    simulatePlayerAction(playerSb->getId(), ActionType::Check);
    simulatePlayerAction(playerBb->getId(), ActionType::Check);
    EXPECT_EQ(myHandFsm->getGameState(), GameState::Turn);

    if (targetState == GameState::Turn)
    {
        return; // We're done
    }

    // Advance to River if needed
    simulatePlayerAction(playerSb->getId(), ActionType::Check);
    simulatePlayerAction(playerBb->getId(), ActionType::Check);
    EXPECT_EQ(myHandFsm->getGameState(), GameState::River);
}

bool ActionsPostflopTest::containsAction(const std::vector<ActionType>& actions, ActionType action)
{
    return std::find(actions.begin(), actions.end(), action) != actions.end();
}

bool ActionsPostflopTest::doesNotContainAction(const std::vector<ActionType>& actions, ActionType action)
{
    return std::find(actions.begin(), actions.end(), action) == actions.end();
}

// ==================== Tests for getValidActionsForPlayer in Postflop States ====================

TEST_F(ActionsPostflopTest, GetValidActionsForPlayerFirstToActCanCheckOrBet)
{
    logTestMessage("Testing valid actions for first to act in postflop (can check or bet)");

    setupBasicHeadsUpScenario();
    advanceToPostflop(GameState::Flop); // Test on Flop (same logic applies to Turn and River)

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto validActions = getValidActionsForPlayer(*myHandFsm, playerSb->getId());

    EXPECT_FALSE(validActions.empty()) << "First to act should have valid actions";

    // First to act should be able to check, bet, or go all-in
    EXPECT_TRUE(containsAction(validActions, ActionType::Check)) << "First to act should be able to check";
    EXPECT_TRUE(containsAction(validActions, ActionType::Bet)) << "First to act should be able to bet";
    EXPECT_TRUE(containsAction(validActions, ActionType::Allin)) << "First to act should be able to go all-in";
    EXPECT_TRUE(containsAction(validActions, ActionType::Fold)) << "First to act should be able to fold";

    // First to act should NOT be able to call or raise (no bet to respond to)
    EXPECT_TRUE(doesNotContainAction(validActions, ActionType::Call)) << "First to act should NOT be able to call";
    EXPECT_TRUE(doesNotContainAction(validActions, ActionType::Raise)) << "First to act should NOT be able to raise";
}

TEST_F(ActionsPostflopTest, GetValidActionsForPlayerAfterBet)
{
    logTestMessage("Testing valid actions for player facing a bet in postflop");

    setupBasicHeadsUpScenario();
    advanceToPostflop(GameState::Flop);

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // SB bets
    simulatePlayerAction(playerSb->getId(), ActionType::Bet, 50);

    auto validActions = getValidActionsForPlayer(*myHandFsm, playerBb->getId());

    EXPECT_FALSE(validActions.empty()) << "Player facing bet should have valid actions";

    // Player facing bet should be able to fold, call, raise, or go all-in
    EXPECT_TRUE(containsAction(validActions, ActionType::Fold)) << "Player should be able to fold";
    EXPECT_TRUE(containsAction(validActions, ActionType::Call)) << "Player should be able to call";
    EXPECT_TRUE(containsAction(validActions, ActionType::Raise)) << "Player should be able to raise";
    EXPECT_TRUE(containsAction(validActions, ActionType::Allin)) << "Player should be able to go all-in";

    // Player facing bet should NOT be able to check or bet
    EXPECT_TRUE(doesNotContainAction(validActions, ActionType::Check))
        << "Player should NOT be able to check when facing bet";
    EXPECT_TRUE(doesNotContainAction(validActions, ActionType::Bet))
        << "Player should NOT be able to bet when facing bet";
}

TEST_F(ActionsPostflopTest, GetValidActionsForPlayerAfterCheck)
{
    logTestMessage("Testing valid actions for player after opponent checks in postflop");

    setupBasicHeadsUpScenario();
    advanceToPostflop(GameState::Turn); // Test on Turn

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 1);

    // SB checks
    simulatePlayerAction(playerSb->getId(), ActionType::Check);

    auto validActions = getValidActionsForPlayer(*myHandFsm, playerBb->getId());

    EXPECT_FALSE(validActions.empty()) << "Player after check should have valid actions";

    // Player after check should be able to check, bet, or go all-in
    EXPECT_TRUE(containsAction(validActions, ActionType::Check)) << "Player should be able to check after check";
    EXPECT_TRUE(containsAction(validActions, ActionType::Bet)) << "Player should be able to bet after check";
    EXPECT_TRUE(containsAction(validActions, ActionType::Allin)) << "Player should be able to go all-in";
    EXPECT_TRUE(containsAction(validActions, ActionType::Fold)) << "Player should be able to fold";

    // Player after check should NOT be able to call or raise
    EXPECT_TRUE(doesNotContainAction(validActions, ActionType::Call))
        << "Player should NOT be able to call after check";
    EXPECT_TRUE(doesNotContainAction(validActions, ActionType::Raise))
        << "Player should NOT be able to raise after check";
}

TEST_F(ActionsPostflopTest, GetValidActionsForPlayerWithLimitedCashPostflop)
{
    logTestMessage("Testing valid actions for player with limited cash in postflop");

    setupBasicHeadsUpScenario();
    advanceToPostflop(GameState::River); // Test on River

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);

    // Set SB to have very little cash
    setupPlayerWithLimitedCash(playerSb->getId(), 25);

    auto validActions = getValidActionsForPlayer(*myHandFsm, playerSb->getId());

    EXPECT_FALSE(validActions.empty()) << "Player with limited cash should still have some valid actions";

    // Player should be able to check, fold, or go all-in
    EXPECT_TRUE(containsAction(validActions, ActionType::Check)) << "Player should be able to check";
    EXPECT_TRUE(containsAction(validActions, ActionType::Fold)) << "Player should be able to fold";
    EXPECT_TRUE(containsAction(validActions, ActionType::Allin)) << "Player should be able to go all-in";

    // Player might be able to bet if they have enough chips
    if (playerSb->getCash() >= myHandFsm->getSmallBlind())
    {
        EXPECT_TRUE(containsAction(validActions, ActionType::Bet))
            << "Player with sufficient cash should be able to bet";
    }
    else
    {
        EXPECT_TRUE(doesNotContainAction(validActions, ActionType::Bet))
            << "Player with insufficient cash should NOT be able to bet";
    }
}

TEST_F(ActionsPostflopTest, GetValidActionsForThreePlayerPostflopScenario)
{
    logTestMessage("Testing valid actions in three-player postflop scenario");

    setupThreePlayerScenario();

    // Advance to flop: dealer folds, SB calls, BB checks
    auto playerDealer = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 1);
    auto playerBb = getPlayerFsmById(myActingPlayersListFsm, 2);

    simulatePlayerAction(playerDealer->getId(), ActionType::Fold);
    simulatePlayerAction(playerSb->getId(), ActionType::Call);
    simulatePlayerAction(playerBb->getId(), ActionType::Check);

    EXPECT_EQ(myHandFsm->getGameState(), GameState::Flop);

    // Test SB's actions (first to act on flop)
    auto validActionsSb = getValidActionsForPlayer(*myHandFsm, playerSb->getId());

    EXPECT_FALSE(validActionsSb.empty()) << "SB should have valid actions on flop";

    // SB should be able to check, bet, fold, or go all-in
    EXPECT_TRUE(containsAction(validActionsSb, ActionType::Check)) << "SB should be able to check";
    EXPECT_TRUE(containsAction(validActionsSb, ActionType::Bet)) << "SB should be able to bet";
    EXPECT_TRUE(containsAction(validActionsSb, ActionType::Fold)) << "SB should be able to fold";
    EXPECT_TRUE(containsAction(validActionsSb, ActionType::Allin)) << "SB should be able to go all-in";

    // SB should NOT be able to call or raise
    EXPECT_TRUE(doesNotContainAction(validActionsSb, ActionType::Call)) << "SB should NOT be able to call";
    EXPECT_TRUE(doesNotContainAction(validActionsSb, ActionType::Raise)) << "SB should NOT be able to raise";
}

// ==================== Tests for validatePlayerAction in Postflop States ====================

TEST_F(ActionsPostflopTest, ValidatePlayerActionCheckValidInPostflop)
{
    logTestMessage("Testing that check action is valid when no bet exists in postflop");

    setupBasicHeadsUpScenario();
    advanceToPostflop(GameState::Flop);

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    PlayerAction checkAction{playerSb->getId(), ActionType::Check, 0};

    bool isValid = validatePlayerAction(myActingPlayersListFsm, checkAction, *myHandFsm->getBettingActions(),
                                        myHandFsm->getSmallBlind(), myHandFsm->getGameState());

    EXPECT_TRUE(isValid) << "Check action should be valid when no bet exists in postflop";
}

TEST_F(ActionsPostflopTest, ValidatePlayerActionBetValidInPostflop)
{
    logTestMessage("Testing that bet action is valid when no bet exists in postflop");

    setupBasicHeadsUpScenario();
    advanceToPostflop(GameState::Turn);

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    PlayerAction betAction{playerSb->getId(), ActionType::Bet, 50};

    bool isValid = validatePlayerAction(myActingPlayersListFsm, betAction, *myHandFsm->getBettingActions(),
                                        myHandFsm->getSmallBlind(), myHandFsm->getGameState());

    EXPECT_TRUE(isValid) << "Bet action should be valid when no bet exists in postflop";
}

// ==================== Integration Tests for Postflop ====================

TEST_F(ActionsPostflopTest, IntegrationPostflopValidActionsMatchValidation)
{
    logTestMessage("Testing that getValidActionsForPlayer results match validatePlayerAction in postflop");

    setupBasicHeadsUpScenario();
    advanceToPostflop(GameState::River);

    auto playerSb = getPlayerFsmById(myActingPlayersListFsm, 0);
    auto validActions = getValidActionsForPlayer(*myHandFsm, playerSb->getId());

    EXPECT_FALSE(validActions.empty()) << "Player should have valid actions in postflop";

    // Test each valid action type
    for (ActionType actionType : validActions)
    {
        int amount = 0;
        if (actionType == ActionType::Bet)
        {
            amount = myHandFsm->getSmallBlind() * 2; // Valid bet amount
        }
        else if (actionType == ActionType::Raise)
        {
            amount =
                myHandFsm->getSmallBlind() * 4; // Valid raise amount (though this shouldn't happen for first to act)
        }
        else if (actionType == ActionType::Allin)
        {
            amount = playerSb->getCash();
        }

        PlayerAction action{playerSb->getId(), actionType, amount};
        bool isValid = validatePlayerAction(myActingPlayersListFsm, action, *myHandFsm->getBettingActions(),
                                            myHandFsm->getSmallBlind(), myHandFsm->getGameState());

        EXPECT_TRUE(isValid) << "Action " << static_cast<int>(actionType)
                             << " should be valid according to validatePlayerAction in postflop";
    }
}

} // namespace pkt::test
