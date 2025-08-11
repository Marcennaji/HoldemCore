// tests/BettingRoundsLegacyTest.cpp

#include "BettingRoundsLegacyTest.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/Helpers.h"
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
    myEvents.clear();

    gameData.maxNumberOfPlayers = MAX_NUMBER_OF_PLAYERS;
    gameData.startMoney = 1000;
    gameData.firstSmallBlind = 10;
    gameData.tableProfile = TableProfile::RandomOpponents;
}

void BettingRoundsLegacyTest::TearDown()
{
    checkPostRiverConditions();
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
    initializeHandForTesting(2, gameData);
    myHand->start();
}
TEST_F(BettingRoundsLegacyTest, StartShouldGoFromPreflopToPostRiver3Players)
{
    initializeHandForTesting(3, gameData);
    myHand->start();
}
TEST_F(BettingRoundsLegacyTest, StartShouldGoFromPreflopToPostRiver6Players)
{
    initializeHandForTesting(6, gameData);
    myHand->start();
}
TEST_F(BettingRoundsLegacyTest, PlayersDoNotActAfterFolding)
{
    initializeHandForTesting(4, gameData);
    myHand->start();

    for (const auto& player : *mySeatsList)
    {
        bool folded = false;
        bool actedAfterFold = false;

        auto checkRound = [&](const std::vector<ActionType>& actions)
        {
            for (const auto& action : actions)
            {
                if (folded)
                    actedAfterFold = true;
                if (action == ActionType::Fold)
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

TEST_F(BettingRoundsLegacyTest, ShouldRecordAllActionsInHandHistoryChronologically)
{
    initializeHandForTesting(3, gameData);
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
            EXPECT_TRUE(action != ActionType::None);
        }
    }
}
TEST_F(BettingRoundsLegacyTest, ActionOrderStartsCorrectlyInHeadsUpPreflop)
{
    initializeHandForTesting(2, gameData);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    const auto& preflop = history.front();
    ASSERT_EQ(preflop.round, Preflop);
    ASSERT_FALSE(preflop.actions.empty());

    auto dealerIt = getPlayerListIteratorById(myHand->getSeatsList(), myHand->getDealerPlayerId());

    ASSERT_FALSE(preflop.actions.empty());
    // in heads-up, preflop, the first player to act is the dealer
    EXPECT_EQ(preflop.actions.front().first, (*dealerIt)->getId());
}

TEST_F(BettingRoundsLegacyTest, FirstToActPostflopIsLeftOfDealer)
{
    initializeHandForTesting(3, gameData);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        if (round.round == Flop || round.round == Turn || round.round == River)
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
    initializeHandForTesting(6, gameData);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        for (const auto& [playerId, action] : round.actions)
        {
            EXPECT_TRUE(getPlayerListIteratorById(myHand->getSeatsList(), playerId) != myHand->getSeatsList()->end());
            EXPECT_NE(action, ActionType::None);
        }
    }
}

TEST_F(BettingRoundsLegacyTest, NoTwoConsecutiveActionsBySamePlayerInRound)
{
    initializeHandForTesting(3, gameData);
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
    initializeHandForTesting(4, gameData);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        if (round.round == Flop || round.round == Turn || round.round == River)
        {

            // Check the first action in the round
            const auto& firstAction = round.actions.front();
            EXPECT_NE(firstAction.second, ActionType::Raise)
                << "Invalid action: Player started the post-flop round with a raise.";
        }
    }
}
TEST_F(BettingRoundsLegacyTest, NoPlayerStartsPostflopRoundByFolding)
{
    initializeHandForTesting(4, gameData);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        if (round.round == Flop || round.round == Turn || round.round == River)
        {
            ASSERT_FALSE(round.actions.empty()) << "Postflop round has no actions.";

            // Check the first action in the round
            const auto& firstAction = round.actions.front();
            EXPECT_NE(firstAction.second, ActionType::Fold)
                << "Invalid action: Player started a postflop round by folding.";
        }
    }
}
TEST_F(BettingRoundsLegacyTest, NoPlayerBetsAfterRaise)
{
    initializeHandForTesting(4, gameData);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        ASSERT_FALSE(round.actions.empty()) << "Round has no actions.";

        // Track the previous action
        pkt::core::ActionType previousAction = ActionType::None;

        for (const auto& [playerId, action] : round.actions)
        {
            if (previousAction == ActionType::Raise)
            {
                EXPECT_NE(action, ActionType::Bet) << "Invalid action: Player placed a bet after a raise.";
            }
            previousAction = action;
        }
    }
}
TEST_F(BettingRoundsLegacyTest, NoPlayerFoldsPostFlopWhenNoBet)
{
    initializeHandForTesting(4, gameData);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        if (round.round == Flop || round.round == Turn || round.round == River)
        {

            bool hasBetOrRaise = false;

            for (const auto& [playerId, action] : round.actions)
            {
                if (action == ActionType::Bet || action == ActionType::Raise)
                {
                    hasBetOrRaise = true;
                }

                if (!hasBetOrRaise)
                {
                    EXPECT_NE(action, ActionType::Fold)
                        << "Invalid action: Player folded when there was no bet or raise.";
                }
            }
        }
    }
}

TEST_F(BettingRoundsLegacyTest, NoConsecutiveRaisesBySamePlayer)
{
    initializeHandForTesting(4, gameData);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        ASSERT_FALSE(round.actions.empty()) << "Round has no actions.";

        unsigned lastRaiserId = std::numeric_limits<unsigned>::max();

        for (const auto& [playerId, action] : round.actions)
        {
            if (action == ActionType::Raise)
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
    initializeHandForTesting(4, gameData);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        ASSERT_FALSE(round.actions.empty()) << "Round has no actions.";

        pkt::core::ActionType previousAction = ActionType::None;

        for (const auto& [playerId, action] : round.actions)
        {
            if (previousAction == ActionType::Bet || previousAction == ActionType::Raise)
            {
                EXPECT_NE(action, ActionType::Check) << "Invalid action: Player checked after a bet or raise.";
            }
            previousAction = action;
        }
    }
}
TEST_F(BettingRoundsLegacyTest, OnlyOneBetAllowedPerRoundUnlessRaised)
{
    initializeHandForTesting(4, gameData);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        int betCount = 0;
        for (const auto& [playerId, action] : round.actions)
        {
            if (action == ActionType::Bet)
                ++betCount;
        }
        EXPECT_LE(betCount, 1) << "Multiple bets occurred in a single round.";
    }
}
TEST_F(BettingRoundsLegacyTest, FoldedPlayerDoesNotReappearInLaterRounds)
{
    initializeHandForTesting(4, gameData);
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

            if (action == ActionType::Fold)
                hasFolded[playerId] = true;
        }
    }
}
TEST_F(BettingRoundsLegacyTest, NoBettingInPostRiverRound)
{
    initializeHandForTesting(4, gameData);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        if (round.round == PostRiver)
        {
            for (const auto& [playerId, action] : round.actions)
            {
                EXPECT_NE(action, ActionType::Bet);
                EXPECT_NE(action, ActionType::Raise);
                EXPECT_NE(action, ActionType::Call);
            }
        }
    }
}
TEST_F(BettingRoundsLegacyTest, AllInPlayerDoesNotActAgain)
{
    initializeHandForTesting(3, gameData);
    auto p = *mySeatsList->begin();
    p->setCash(0); // Force all-in
    p->setAction(ActionType::Allin);

    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        for (const auto& [playerId, action] : round.actions)
        {
            if (playerId == p->getId())
                EXPECT_NE(action, ActionType::None);
        }
    }
}
TEST_F(BettingRoundsLegacyTest, NoExtraActionsAfterFinalCall)
{
    initializeHandForTesting(3, gameData);
    myHand->start();

    const auto& history = myHand->getHandActionHistory();
    for (const auto& round : history)
    {
        bool raiseSeen = false;
        int callsAfterRaise = 0;
        for (const auto& [_, action] : round.actions)
        {
            if (raiseSeen && action == ActionType::Call)
                ++callsAfterRaise;
            if (action == ActionType::Raise)
                raiseSeen = true;
        }
        if (raiseSeen)
            EXPECT_LE(callsAfterRaise, static_cast<int>(mySeatsList->size()) - 1);
    }
}
TEST_F(BettingRoundsLegacyTest, HeadsUpEndsImmediatelyOnFold)
{
    initializeHandForTesting(2, gameData);
    (*mySeatsList->begin())->setAction(ActionType::Fold);
    myHand->start();

    // If one folds, there should only be preflop actions
    const auto& history = myHand->getHandActionHistory();
    EXPECT_EQ(history.size(), 0);
}

} // namespace pkt::test
