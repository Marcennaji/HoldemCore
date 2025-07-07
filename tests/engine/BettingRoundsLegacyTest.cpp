// tests/BettingRoundsLegacyTest.cpp

#include "BettingRoundsLegacyTest.h"
#include "core/engine/model/PlayerAction.h"
#include "core/services/GlobalServices.h"

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{
void BettingRoundsLegacyTest::logTestMessage(const std::string& message) const
{
    GlobalServices::instance().logger()->verbose("BettingRoundsLegacyTest : " + message);
}

void BettingRoundsLegacyTest::SetUp()
{
    EngineTest::SetUp();

    setFlowMode(pkt::core::FlowMode::Legacy);

    // events for game flow, in the legacy code (that has an unwanted dependencies in which the gui sometimes pilots the
    // game). These dependencies will be removed in the new engine code, which will use a FSM for the betting rounds
    // handling.
    myEvents.clear();
    myEvents.onActivePlayerActionDone = [this]() { myHand->resolveHandConditions(); };
    myEvents.onBettingRoundAnimation = [this](int bettingRoundId) { bettingRoundAnimation(bettingRoundId); };
    myEvents.onDealBettingRoundCards = [this](int bettingRoundId) { dealBettingRoundCards(bettingRoundId); };
    myEvents.onStartPreflop = [this]() { myHand->getCurrentBettingRound()->run(); };
    myEvents.onStartFlop = [this]() { myHand->getCurrentBettingRound()->run(); };
    myEvents.onStartTurn = [this]() { myHand->getCurrentBettingRound()->run(); };
    myEvents.onStartRiver = [this]() { myHand->getCurrentBettingRound()->run(); };
    myEvents.onStartPostRiver = [this]() { myHand->getCurrentBettingRound()->run(); };
}

void BettingRoundsLegacyTest::bettingRoundAnimation(int bettingRoundId)
{
    if (bettingRoundId < 4)
    {
        myHand->getCurrentBettingRound()->giveActionToNextBotPlayer();
    }
}

void BettingRoundsLegacyTest::dealBettingRoundCards(int bettingRoundId)
{
    if (bettingRoundId != 0)
    {
        myHand->resolveHandConditions();
    }
}
bool BettingRoundsLegacyTest::isPlayerStillActive(unsigned id) const
{
    for (const auto& p : *myHand->getRunningPlayersList())
    {
        if (p->getId() == id)
            return true;
    }
    return false;
}

// Tests for betting rounds and transitions

TEST_F(BettingRoundsLegacyTest, StartShouldGoFromPreflopToPostRiverHeadsUp)
{
    initializeHandForTesting(2);
    myHand->start();
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStatePostRiver);
}
TEST_F(BettingRoundsLegacyTest, StartShouldGoFromPreflopToPostRiver3Players)
{
    initializeHandForTesting(3);
    myHand->start();
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStatePostRiver);
}
TEST_F(BettingRoundsLegacyTest, StartShouldGoFromPreflopToPostRiver6Players)
{
    initializeHandForTesting(6);
    myHand->start();
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStatePostRiver);
}
TEST_F(BettingRoundsLegacyTest, PlayersDoNotActAfterFolding)
{
    initializeHandForTesting(4);
    myHand->start();

    for (const auto& player : *mySeatsList)
    {
        bool folded = false;
        bool actedAfterFold = false;

        auto checkRound = [&](const std::vector<PlayerAction>& actions)
        {
            for (const auto& action : actions)
            {
                if (folded)
                    actedAfterFold = true;
                if (action == PlayerActionFold)
                    folded = true;
            }
        };

        checkRound(player->getCurrentHandActions().getPreflopActions());
        checkRound(player->getCurrentHandActions().getFlopActions());
        checkRound(player->getCurrentHandActions().getTurnActions());
        checkRound(player->getCurrentHandActions().getRiverActions());

        EXPECT_FALSE(actedAfterFold) << "Player " << player->getId() << " acted after folding.";
    }
}

TEST_F(BettingRoundsLegacyTest, EachPLayerHasAtLeastOneAction)
{
    initializeHandForTesting(4);
    myHand->start();

    size_t totalActionCount = 0;
    for (const auto& player : *mySeatsList)
    {
        totalActionCount += player->getCurrentHandActions().getPreflopActions().size();
        totalActionCount += player->getCurrentHandActions().getFlopActions().size();
        totalActionCount += player->getCurrentHandActions().getTurnActions().size();
        totalActionCount += player->getCurrentHandActions().getRiverActions().size();
    }

    EXPECT_GT(totalActionCount, 0u) << "No actions occurred in any round.";
}

TEST_F(BettingRoundsLegacyTest, ShouldRecordAllActionsInHandHistoryChronologically)
{
    initializeHandForTesting(3);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    EXPECT_FALSE(history.empty()) << "Hand action history should not be empty.";

    for (const auto& roundHistory : history)
    {
        SCOPED_TRACE("Round: " + std::to_string(static_cast<int>(roundHistory.round)));
        EXPECT_FALSE(roundHistory.actions.empty())
            << "No actions recorded for round " << static_cast<int>(roundHistory.round);

        for (const auto& [playerId, action] : roundHistory.actions)
        {
            EXPECT_GE(playerId, 0u);
            EXPECT_TRUE(action != PlayerAction::PlayerActionNone);
        }
    }
}
TEST_F(BettingRoundsLegacyTest, ActionOrderStartsCorrectlyInHeadsUpPreflop)
{
    initializeHandForTesting(2);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    const auto& preflop = history.front();
    ASSERT_EQ(preflop.round, GameStatePreflop);
    ASSERT_FALSE(preflop.actions.empty());

    PlayerListIterator dealerIt = myHand->getPlayerSeatFromId(myHand->getDealerPlayerId());
    ASSERT_FALSE(preflop.actions.empty());
    // in heads-up, preflop, the first player to act is the dealer
    EXPECT_EQ(preflop.actions.front().first, (*dealerIt)->getId());
}

TEST_F(BettingRoundsLegacyTest, FirstToActPostflopIsLeftOfDealer)
{
    initializeHandForTesting(3);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        if (round.round == GameStateFlop || round.round == GameStateTurn || round.round == GameStateRiver)
        {
            ASSERT_FALSE(round.actions.empty());
            unsigned dealerId = myHand->getDealerPlayerId();

            // Get seat iterator for the dealer
            auto seats = myHand->getSeatsList();
            auto it = std::find_if(seats->begin(), seats->end(),
                                   [dealerId](const auto& p) { return p->getId() == dealerId; });

            // Move to the next seat (left of dealer)
            ++it;
            if (it == seats->end())
                it = seats->begin();

            // Collect all players who acted in this round
            std::set<unsigned> actors;
            for (const auto& [pid, action] : round.actions)
                actors.insert(pid);

            // Find first player who acted clockwise from dealer
            unsigned expectedFirstId = std::numeric_limits<unsigned>::max();
            for (size_t i = 0; i < seats->size(); ++i)
            {
                unsigned id = (*it)->getId();
                if (actors.count(id))
                {
                    expectedFirstId = id;
                    break;
                }
                ++it;
                if (it == seats->end())
                    it = seats->begin();
            }

            ASSERT_NE(expectedFirstId, std::numeric_limits<unsigned>::max())
                << "No valid first actor found from seat order";

            EXPECT_EQ(round.actions.front().first, expectedFirstId)
                << "First action in round " << static_cast<int>(round.round)
                << " not from first eligible player after dealer";
        }
    }
}

TEST_F(BettingRoundsLegacyTest, AllActionsAreFromActivePlayersOnly)
{
    initializeHandForTesting(6);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        for (const auto& [playerId, action] : round.actions)
        {
            EXPECT_TRUE(myHand->getPlayerSeatFromId(playerId) != myHand->getSeatsList()->end());
            EXPECT_NE(action, PlayerAction::PlayerActionNone);
        }
    }
}

TEST_F(BettingRoundsLegacyTest, NoTwoConsecutiveActionsBySamePlayerInRound)
{
    initializeHandForTesting(3);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        unsigned lastId = std::numeric_limits<unsigned>::max();
        for (const auto& [playerId, action] : round.actions)
        {
            EXPECT_NE(playerId, lastId);
            lastId = playerId;
        }
    }
}
TEST_F(BettingRoundsLegacyTest, NoPlayerStartsPostFlopRoundWithRaise)
{
    initializeHandForTesting(4);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        if (round.round == GameStateFlop || round.round == GameStateTurn || round.round == GameStateRiver)
        {

            // Check the first action in the round
            const auto& firstAction = round.actions.front();
            EXPECT_NE(firstAction.second, PlayerAction::PlayerActionRaise)
                << "Invalid action: Player started the post-flop round with a raise.";
        }
    }
}
TEST_F(BettingRoundsLegacyTest, NoPlayerStartsPostflopRoundByFolding)
{
    initializeHandForTesting(4);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        if (round.round == GameStateFlop || round.round == GameStateTurn || round.round == GameStateRiver)
        {
            ASSERT_FALSE(round.actions.empty()) << "Postflop round has no actions.";

            // Check the first action in the round
            const auto& firstAction = round.actions.front();
            EXPECT_NE(firstAction.second, PlayerAction::PlayerActionFold)
                << "Invalid action: Player started a postflop round by folding.";
        }
    }
}
TEST_F(BettingRoundsLegacyTest, NoPlayerBetsAfterRaise)
{
    initializeHandForTesting(4);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        ASSERT_FALSE(round.actions.empty()) << "Round has no actions.";

        // Track the previous action
        pkt::core::PlayerAction previousAction = PlayerAction::PlayerActionNone;

        for (const auto& [playerId, action] : round.actions)
        {
            if (previousAction == PlayerAction::PlayerActionRaise)
            {
                EXPECT_NE(action, PlayerAction::PlayerActionBet)
                    << "Invalid action: Player placed a bet after a raise.";
            }
            previousAction = action;
        }
    }
}
TEST_F(BettingRoundsLegacyTest, NoPlayerFoldsPostFlopWhenNoBet)
{
    initializeHandForTesting(4);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        if (round.round == GameStateFlop || round.round == GameStateTurn || round.round == GameStateRiver)
        {

            bool hasBetOrRaise = false;

            for (const auto& [playerId, action] : round.actions)
            {
                if (action == PlayerAction::PlayerActionBet || action == PlayerAction::PlayerActionRaise)
                {
                    hasBetOrRaise = true;
                }

                if (!hasBetOrRaise)
                {
                    EXPECT_NE(action, PlayerAction::PlayerActionFold)
                        << "Invalid action: Player folded when there was no bet or raise.";
                }
            }
        }
    }
}
TEST_F(BettingRoundsLegacyTest, NoPlayerCallsWithoutBetOrRaise)
{
    initializeHandForTesting(4);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        ASSERT_FALSE(round.actions.empty()) << "Round has no actions.";

        bool hasBetOrRaise = false;

        for (const auto& [playerId, action] : round.actions)
        {
            if (action == PlayerAction::PlayerActionBet || action == PlayerAction::PlayerActionRaise)
            {
                hasBetOrRaise = true;
            }

            if (!hasBetOrRaise)
            {
                EXPECT_NE(action, PlayerAction::PlayerActionCall)
                    << "Invalid action: Player called when there was no bet or raise.";
            }
        }
    }
}
TEST_F(BettingRoundsLegacyTest, NoConsecutiveRaisesBySamePlayer)
{
    initializeHandForTesting(4);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        ASSERT_FALSE(round.actions.empty()) << "Round has no actions.";

        unsigned lastRaiserId = std::numeric_limits<unsigned>::max();

        for (const auto& [playerId, action] : round.actions)
        {
            if (action == PlayerAction::PlayerActionRaise)
            {
                EXPECT_NE(playerId, lastRaiserId)
                    << "Invalid action: Player raised consecutively without another player acting.";
                lastRaiserId = playerId;
            }
        }
    }
}
TEST_F(BettingRoundsLegacyTest, NoPlayerChecksAfterBetOrRaise)
{
    initializeHandForTesting(4);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        ASSERT_FALSE(round.actions.empty()) << "Round has no actions.";

        pkt::core::PlayerAction previousAction = PlayerAction::PlayerActionNone;

        for (const auto& [playerId, action] : round.actions)
        {
            if (previousAction == PlayerAction::PlayerActionBet || previousAction == PlayerAction::PlayerActionRaise)
            {
                EXPECT_NE(action, PlayerAction::PlayerActionCheck)
                    << "Invalid action: Player checked after a bet or raise.";
            }
            previousAction = action;
        }
    }
}
TEST_F(BettingRoundsLegacyTest, OnlyOneBetAllowedPerRoundUnlessRaised)
{
    initializeHandForTesting(4);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        int betCount = 0;
        for (const auto& [playerId, action] : round.actions)
        {
            if (action == PlayerAction::PlayerActionBet)
                ++betCount;
        }
        EXPECT_LE(betCount, 1) << "Multiple bets occurred in a single round.";
    }
}
TEST_F(BettingRoundsLegacyTest, FoldedPlayerDoesNotReappearInLaterRounds)
{
    initializeHandForTesting(4);
    myHand->start();

    std::unordered_map<unsigned, bool> hasFolded;

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        for (const auto& [playerId, action] : round.actions)
        {
            if (hasFolded[playerId])
            {
                FAIL() << "Player " << playerId << " acted after folding.";
            }

            if (action == PlayerAction::PlayerActionFold)
                hasFolded[playerId] = true;
        }
    }
}
TEST_F(BettingRoundsLegacyTest, NoBettingInPostRiverRound)
{
    initializeHandForTesting(4);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        if (round.round == GameStatePostRiver)
        {
            for (const auto& [playerId, action] : round.actions)
            {
                EXPECT_NE(action, PlayerAction::PlayerActionBet);
                EXPECT_NE(action, PlayerAction::PlayerActionRaise);
                EXPECT_NE(action, PlayerAction::PlayerActionCall);
            }
        }
    }
}
TEST_F(BettingRoundsLegacyTest, AllInPlayerDoesNotActAgain)
{
    initializeHandForTesting(3);
    auto p = *mySeatsList->begin();
    p->setCash(0); // Force all-in
    p->setAction(PlayerAction::PlayerActionAllin);

    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        for (const auto& [playerId, action] : round.actions)
        {
            if (playerId == p->getId())
                EXPECT_NE(action, PlayerAction::PlayerActionNone);
        }
    }
}
TEST_F(BettingRoundsLegacyTest, NoExtraActionsAfterFinalCall)
{
    initializeHandForTesting(3);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        bool raiseSeen = false;
        int callsAfterRaise = 0;
        for (const auto& [_, action] : round.actions)
        {
            if (raiseSeen && action == PlayerAction::PlayerActionCall)
                ++callsAfterRaise;
            if (action == PlayerAction::PlayerActionRaise)
                raiseSeen = true;
        }
        if (raiseSeen)
            EXPECT_LE(callsAfterRaise, static_cast<int>(mySeatsList->size()) - 1);
    }
}
TEST_F(BettingRoundsLegacyTest, HeadsUpEndsImmediatelyOnFold)
{
    initializeHandForTesting(2);
    (*mySeatsList->begin())->setAction(PlayerAction::PlayerActionFold);
    myHand->start();

    // If one folds, there should only be preflop actions
    const auto& history = myHand->getHandActionHistory();
    EXPECT_EQ(history.size(), 0);
}

} // namespace pkt::test
