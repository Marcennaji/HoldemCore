// tests/BettingRoundsLegacyTest.cpp

#include "BettingRoundsLegacyTest.h"
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

    // Essential events for game flow, in the legacy code (that has an unwanted dependency in which the gui pilots the
    // game)
    myEvents.onActivePlayerActionDone = [this]() { myHand->resolveHandConditions(); };
    myEvents.onBettingRoundAnimation = [this](int bettingRoundId) { bettingRoundAnimation(bettingRoundId); };
    myEvents.onDealBettingRoundCards = [this](int bettingRoundId) { dealBettingRoundCards(bettingRoundId); };
    myEvents.onStartPreflop = [this]() { myHand->getCurrentBettingRound()->run(); };
    myEvents.onStartFlop = [this]() { myHand->getCurrentBettingRound()->run(); };
    myEvents.onStartTurn = [this]() { myHand->getCurrentBettingRound()->run(); };
    myEvents.onStartRiver = [this]() { myHand->getCurrentBettingRound()->run(); };
    myEvents.onStartPostRiver = [this]() { myHand->getCurrentBettingRound()->postRiverRun(); };
}

void BettingRoundsLegacyTest::bettingRoundAnimation(int bettingRoundID)
{
    if (bettingRoundID < 4)
    {
        myHand->getCurrentBettingRound()->nextPlayer();
    }
}

void BettingRoundsLegacyTest::dealBettingRoundCards(int bettingRoundId)
{
    if (bettingRoundId != 0)
    {
        myHand->resolveHandConditions();
    }
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

TEST_F(BettingRoundsLegacyTest, EachRoundHasAtLeastOneActionIfNotAllIn)
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

    EXPECT_GT(totalActionCount, 0u) << "No betting actions occurred in any round.";
}

TEST_F(BettingRoundsLegacyTest, DISABLED_PreflopActionOrderMatchesSeatOrder)
{
    initializeHandForTesting(3);
    myHand->start();

    std::vector<int> actionOrder;

    for (const auto& player : *mySeatsList)
    {
        for (const auto& action : player->getCurrentHandActions().getPreflopActions())
        {
            actionOrder.push_back(player->getId());
            break;
        }
    }

    auto it = mySeatsList->begin();
    std::advance(it, 2);
    if (it == mySeatsList->end())
        it = mySeatsList->begin();

    std::vector<int> expectedOrder;
    for (size_t i = 0; i < mySeatsList->size(); ++i)
    {
        expectedOrder.push_back((*it)->getId());
        ++it;
        if (it == mySeatsList->end())
            it = mySeatsList->begin();
    }

    EXPECT_EQ(actionOrder, expectedOrder) << "Preflop action order does not match expected seat order.";
}

} // namespace pkt::test
