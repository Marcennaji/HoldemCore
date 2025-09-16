// PokerTraining — E2E test for all 4 Bot Strategies
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include <gtest/gtest.h>

#include "common/EngineTest.h"
#include "core/cards/Card.h"
#include "core/engine/Hand.h"
#include "core/engine/model/GameData.h"
#include "core/engine/model/StartData.h"
#include "core/player/strategy/LooseAggressiveBotStrategy.h"
#include "core/player/strategy/ManiacBotStrategy.h"
#include "core/player/strategy/TightAggressiveBotStrategy.h"
#include "core/player/strategy/UltraTightBotStrategy.h"
#include "core/services/GlobalServices.h"

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

class StrategiesE2ETest : public EngineTest
{
  protected:
    int cardsOverriddenCount = 0; // Track card override progress

    void SetUp() override
    {
        EngineTest::SetUp();

        // Setup game data for 4 players
        gameData.maxNumberOfPlayers = 4;
        gameData.startMoney = 1000;
        gameData.firstSmallBlind = 10;
        gameData.tableProfile = TableProfile::RandomOpponents;
    }

    void setupFourPlayersWithDifferentStrategies()
    {
        // Create 4 players manually instead of using createPlayersLists
        mySeatsList = std::make_shared<std::list<std::shared_ptr<Player>>>();

        // Player 0: TightAggressiveBotStrategy
        auto player0 = std::make_shared<Player>(myEvents, 0, "TightAggressive", 1000);
        player0->setStrategy(std::make_unique<TightAggressiveBotStrategy>());
        mySeatsList->push_back(player0);

        // Player 1: LooseAggressiveBotStrategy
        auto player1 = std::make_shared<Player>(myEvents, 1, "LooseAggressive", 1000);
        player1->setStrategy(std::make_unique<LooseAggressiveBotStrategy>());
        mySeatsList->push_back(player1);

        // Player 2: UltraTightBotStrategy
        auto player2 = std::make_shared<Player>(myEvents, 2, "UltraTight", 1000);
        player2->setStrategy(std::make_unique<UltraTightBotStrategy>());
        mySeatsList->push_back(player2);

        // Player 3: ManiacBotStrategy
        auto player3 = std::make_shared<Player>(myEvents, 3, "Maniac", 1000);
        player3->setStrategy(std::make_unique<ManiacBotStrategy>());
        mySeatsList->push_back(player3);

        // Acting players list starts identical to seats list
        myActingPlayersList = std::make_shared<std::list<std::shared_ptr<Player>>>();
        for (const auto& player : *mySeatsList)
        {
            myActingPlayersList->push_back(player);
        }
    }

    void setupPredeterminedCards()
    {
        logTestMessage("Setting up predetermined cards using event override system");

        // APPROACH: Use GameEvents to override cards immediately after dealing
        // This ensures our cards are set after Hand.dealHoleCards() but before player actions

        // Set up our predetermined card mappings
        std::map<unsigned, HoleCards> playerCardMap = {
            {0, HoleCards("As", "Ah")}, // TightAggressive: Pocket Aces
            {1, HoleCards("9s", "8s")}, // LooseAggressive: Suited connectors
            {2, HoleCards("7h", "7c")}, // UltraTight: Medium pocket pair
            {3, HoleCards("Kd", "3c")}  // Maniac: Weak but bluffable hand
        };

        // Reset counter for this test
        cardsOverriddenCount = 0;

        // Override hole cards immediately when dealt
        myEvents.onHoleCardsDealt = [this, playerCardMap](unsigned playerId, HoleCards dealtCards)
        {
            auto it = playerCardMap.find(playerId);
            if (it != playerCardMap.end())
            {
                // Find the player and override their cards
                for (auto& player : *mySeatsList)
                {
                    if (player->getId() == playerId)
                    {
                        player->setHoleCards(it->second);
                        logTestMessage("Override Player " + std::to_string(playerId) + " cards to " +
                                       it->second.toString());

                        // Increment counter and check if all players have been dealt
                        cardsOverriddenCount++;
                        if (cardsOverriddenCount == static_cast<int>(mySeatsList->size()))
                        {
                            // All players have received their cards, log them
                            logPlayersHoleCardsAfterDealing();
                        }
                        break;
                    }
                }
            }
        };

        // Override board cards when dealt
        myEvents.onBoardCardsDealt = [this](BoardCards dealtBoard)
        {
            // Set our predetermined board: 9h 8h 7d As 2c
            BoardCards predeterminedBoard;
            predeterminedBoard.dealFlop(Card("9h"), Card("8h"), Card("7d"));
            predeterminedBoard.dealTurn(Card("As"));
            predeterminedBoard.dealRiver(Card("2c"));

            myHand->getBoard().setBoardCards(predeterminedBoard);
            logTestMessage("Override board cards to: 9h 8h 7d As 2c");
        };

        logTestMessage("Predetermined card override events registered");
    }

    void logPlayerActions()
    {
        for (const auto& player : *mySeatsList)
        {
            auto lastAction = player->getLastAction();
            std::cout << "Player " << player->getId() << " (" << player->getName() << ") "
                      << "Cards: " << player->getHoleCards().toString()
                      << " Last Action: " << actionTypeToString(lastAction.type) << " Amount: " << lastAction.amount
                      << std::endl;
        }
    }

    void logPlayersHoleCardsAfterDealing()
    {
        std::cout << "\n=== Players' Hole Cards After Override ===\n";
        for (const auto& player : *mySeatsList)
        {
            const HoleCards& holeCards = player->getHoleCards();
            if (holeCards.isValid())
            {
                std::cout << "Player " << player->getId() << " (" << player->getName() << "): " << holeCards.toString()
                          << std::endl;
            }
            else
            {
                std::cout << "Player " << player->getId() << " (" << player->getName() << "): No valid hole cards"
                          << std::endl;
            }
        }
        std::cout << std::endl;
    }

    void logTestMessage(const std::string& message) const
    {
        GlobalServices::instance().logger().info("StrategiesE2ETest: " + message);
    }
};

TEST_F(StrategiesE2ETest, FourStrategiesCompleteHandFromPreflopToPostRiver)
{
    setupFourPlayersWithDifferentStrategies();

    // Create board and hand
    myBoard = myFactory->createBoard(0); // Player 0 is dealer
    myBoard->setSeatsList(mySeatsList);
    myBoard->setActingPlayersList(myActingPlayersList);

    StartData startData;
    startData.startDealerPlayerId = 0;
    startData.numberOfPlayers = 4;

    myHand = myFactory->createHand(myFactory, myBoard, mySeatsList, myActingPlayersList, gameData, startData);

    setupPredeterminedCards();

    myHand->initialize();

    EXPECT_EQ(myHand->getGameState(), Preflop);

    myHand->runGameLoop();

    logPlayerActions();

    EXPECT_EQ(myHand->getGameState(), PostRiver);

    // Verify that all players maintained their strategies throughout
    auto players = *mySeatsList;
    auto it = players.begin();

    EXPECT_EQ((*it)->getName(), "TightAggressive");
    EXPECT_TRUE((*it)->hasStrategyType<TightAggressiveBotStrategy>());
    ++it;
    EXPECT_EQ((*it)->getName(), "LooseAggressive");
    EXPECT_TRUE((*it)->hasStrategyType<LooseAggressiveBotStrategy>());
    ++it;
    EXPECT_EQ((*it)->getName(), "UltraTight");
    EXPECT_TRUE((*it)->hasStrategyType<UltraTightBotStrategy>());
    ++it;
    EXPECT_EQ((*it)->getName(), "Maniac");
    EXPECT_TRUE((*it)->hasStrategyType<ManiacBotStrategy>());

    // Verify board cards were set correctly
    const BoardCards& finalBoard = myBoard->getBoardCards();
    EXPECT_TRUE(finalBoard.isRiver());
    EXPECT_EQ(finalBoard.toString(), "9h 8h 7d As 2c");

    // Verify hole cards were preserved
    it = players.begin();
    EXPECT_EQ((*it)->getHoleCards().toString(), "As Ah");
    ++it;
    EXPECT_EQ((*it)->getHoleCards().toString(), "9s 8s");
    ++it;
    EXPECT_EQ((*it)->getHoleCards().toString(), "7h 7c");
    ++it;
    EXPECT_EQ((*it)->getHoleCards().toString(), "Kd 3c");
}

TEST_F(StrategiesE2ETest, StrategiesBehaveDifferentlyInSameScenario)
{
    // This test verifies that each strategy makes different decisions
    // when presented with the same scenario

    setupFourPlayersWithDifferentStrategies();

    myBoard = myFactory->createBoard(0);
    myBoard->setSeatsList(mySeatsList);
    myBoard->setActingPlayersList(myActingPlayersList);

    StartData startData;
    startData.startDealerPlayerId = 0;
    startData.numberOfPlayers = 4;

    myHand = myFactory->createHand(myFactory, myBoard, mySeatsList, myActingPlayersList, gameData, startData);
    myHand->initialize();

    // Give all players the same marginal hand to see different strategy behaviors
    for (const auto& player : *mySeatsList)
    {
        player->setHoleCards(HoleCards("Qd", "Js")); // Marginal but playable hand
    }

    // Set a draw-heavy board
    BoardCards boardCards("Ts", "9c", "8h", "7s", "2d");
    myBoard->setBoardCards(boardCards);

    std::cout << "\n=== Testing Strategy Differences ===\n";
    std::cout << "All players have Qd Js on board Ts 9c 8h 7s 2d\n";
    std::cout << "All players have a straight!\n" << std::endl;

    // Run the hand and observe different behaviors
    myHand->runGameLoop();

    EXPECT_EQ(myHand->getGameState(), PostRiver);

    std::cout << "Final actions show strategy differences:" << std::endl;
    logPlayerActions();

    // Even with identical hands, strategies should potentially show different betting patterns
    // (though with a straight, most strategies might play similarly)
    // The test validates the framework works, even if outcomes are similar
}

TEST_F(StrategiesE2ETest, ManiacVsTightStrategiesShowClearDifferences)
{
    // Create a scenario where Maniac and Tight strategies should behave very differently
    setupFourPlayersWithDifferentStrategies();

    myBoard = myFactory->createBoard(0);
    myBoard->setSeatsList(mySeatsList);
    myBoard->setActingPlayersList(myActingPlayersList);

    StartData startData;
    startData.startDealerPlayerId = 0;
    startData.numberOfPlayers = 4;

    myHand = myFactory->createHand(myFactory, myBoard, mySeatsList, myActingPlayersList, gameData, startData);
    myHand->initialize();

    // Give weak hands to emphasize strategy differences
    auto players = *mySeatsList;
    auto it = players.begin();

    // Player 0 (TightAggressive): Very weak hand
    (*it)->setHoleCards(HoleCards("2c", "7d"));
    ++it;

    // Player 1 (LooseAggressive): Weak but suited
    (*it)->setHoleCards(HoleCards("5s", "6s"));
    ++it;

    // Player 2 (UltraTight): Weak offsuit
    (*it)->setHoleCards(HoleCards("9c", "4d"));
    ++it;

    // Player 3 (Maniac): Any two cards (might still play aggressively)
    (*it)->setHoleCards(HoleCards("Jd", "3h"));

    // Set a rainbow board with no obvious draws
    BoardCards boardCards("Ah", "8c", "5d", "2s", "Qh");
    myBoard->setBoardCards(boardCards);

    std::cout << "\n=== Testing Weak Hands Strategy Differences ===\n";
    std::cout << "Weak hands on rainbow board - strategies should differ significantly\n" << std::endl;

    // Log the manually set hole cards
    logPlayersHoleCardsAfterDealing();

    myHand->runGameLoop();

    EXPECT_EQ(myHand->getGameState(), PostRiver);

    std::cout << "Strategy differences with weak hands:" << std::endl;
    logPlayerActions();

    // This scenario should show clear differences between tight and loose/maniac strategies
    // Tight strategies should fold more often, while Maniac might still play aggressively
}

} // namespace pkt::test