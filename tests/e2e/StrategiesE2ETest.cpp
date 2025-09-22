// HoldemCore — E2E test for all 4 Bot Strategies
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include <gtest/gtest.h>
#include <optional>

#include <iostream>
#include "common/EngineTest.h"
#include "core/engine/cards/Card.h"
#include "core/engine/hand/Hand.h"
#include "core/engine/model/GameData.h"
#include "core/engine/model/StartData.h"
#include "core/player/strategy/LooseAggressiveBotStrategy.h"
#include "core/player/strategy/ManiacBotStrategy.h"
#include "core/player/strategy/TightAggressiveBotStrategy.h"
#include "core/player/strategy/UltraTightBotStrategy.h"

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

// Enum for strategy types to make configuration easier
enum class StrategyType
{
    TightAggressive,
    LooseAggressive,
    UltraTight,
    Maniac
};

// Configuration for a single player
struct PlayerConfig
{
    unsigned playerId;
    std::string playerName;
    StrategyType strategyType;
    HoleCards holeCards;
    int startingCash;

    PlayerConfig(unsigned id, const std::string& name, StrategyType strategy, const HoleCards& cards, int cash = 1000)
        : playerId(id), playerName(name), strategyType(strategy), holeCards(cards), startingCash(cash)
    {
    }
};

// Configuration for board cards
struct BoardConfig
{
    std::vector<Card> flopCards; // 3 cards
    Card turnCard;               // 1 card
    Card riverCard;              // 1 card

    BoardConfig(const std::vector<Card>& flop, const Card& turn, const Card& river)
        : flopCards(flop), turnCard(turn), riverCard(river)
    {
        if (flop.size() != 3)
        {
            throw std::invalid_argument("Flop must have exactly 3 cards");
        }
    }

    // Constructor with string representations
    BoardConfig(const std::string& flop1, const std::string& flop2, const std::string& flop3, const std::string& turn,
                const std::string& river)
        : flopCards({Card(flop1), Card(flop2), Card(flop3)}), turnCard(turn), riverCard(river)
    {
    }
};

// Complete test scenario configuration
struct TestScenario
{
    std::string scenarioName;
    std::vector<PlayerConfig> players;
    BoardConfig boardCards;
    unsigned dealerPlayerId;
    int smallBlind;
    std::string expectedOutcome; // Optional description for validation

    TestScenario(const std::string& name, const std::vector<PlayerConfig>& playerConfigs, const BoardConfig& board,
                 unsigned dealer = 0, int sb = 10, const std::string& expected = "")
        : scenarioName(name), players(playerConfigs), boardCards(board), dealerPlayerId(dealer), smallBlind(sb),
          expectedOutcome(expected)
    {
    }
};

class StrategiesE2ETest : public EngineTest
{
  protected:
    int cardsOverriddenCount = 0;                // Track card override progress
    std::optional<TestScenario> currentScenario; // Optional to avoid default constructor issues

    void SetUp() override
    {
        EngineTest::SetUp();

        // Setup game data - will be updated per scenario
        gameData.maxNumberOfPlayers = 4;
        gameData.startMoney = 1000;
        gameData.firstSmallBlind = 10;
        gameData.tableProfile = TableProfile::RandomOpponents;
    }

    // Factory method to create strategy based on type
    std::unique_ptr<BotStrategyBase> createStrategy(StrategyType strategyType)
    {
        switch (strategyType)
        {
        case StrategyType::TightAggressive:
            return std::make_unique<TightAggressiveBotStrategy>(getServices());
        case StrategyType::LooseAggressive:
            return std::make_unique<LooseAggressiveBotStrategy>(getServices());
        case StrategyType::UltraTight:
            return std::make_unique<UltraTightBotStrategy>(getServices());
        case StrategyType::Maniac:
            return std::make_unique<ManiacBotStrategy>(getServices());
        default:
            throw std::invalid_argument("Unknown strategy type");
        }
    }

    // Setup players based on scenario configuration
    void setupPlayersFromScenario(const TestScenario& scenario)
    {
        currentScenario = scenario;

        // Update game data based on scenario
        gameData.maxNumberOfPlayers = static_cast<int>(scenario.players.size());
        gameData.firstSmallBlind = scenario.smallBlind;

        // Create seats list
        mySeatsList = std::make_shared<std::list<std::shared_ptr<Player>>>();

        for (const auto& playerConfig : scenario.players)
        {
            auto player = std::make_shared<Player>(myEvents, getServices(), playerConfig.playerId,
                                                   playerConfig.playerName, playerConfig.startingCash);
            player->setStrategy(createStrategy(playerConfig.strategyType));
            mySeatsList->push_back(player);
        }

        // Acting players list starts identical to seats list
        myActingPlayersList = std::make_shared<std::list<std::shared_ptr<Player>>>();
        for (const auto& player : *mySeatsList)
        {
            myActingPlayersList->push_back(player);
        }
    }

    // Setup predetermined cards based on scenario
    void setupPredeterminedCardsFromScenario()
    {
        if (!currentScenario.has_value())
        {
            throw std::runtime_error("No current scenario set");
        }

        const auto& scenario = currentScenario.value();
        logTestMessage("Setting up predetermined cards for scenario: " + scenario.scenarioName);

        // Create mapping from player ID to hole cards
        std::map<unsigned, HoleCards> playerCardMap;
        for (const auto& playerConfig : scenario.players)
        {
            playerCardMap[playerConfig.playerId] = playerConfig.holeCards;
        }

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
                        logTestMessage("Override Player " + std::to_string(playerId) + " (" + player->getName() +
                                       ") cards to " + it->second.toString());

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
            if (!currentScenario.has_value())
                return;

            const auto& scenario = currentScenario.value();

            // Check the current game state to determine which cards to set
            GameState currentState = myHand->getGameState();

            BoardCards predeterminedBoard;

            if (currentState == GameState::Flop)
            {
                // Only set flop cards (3 cards)
                predeterminedBoard.dealFlop(scenario.boardCards.flopCards[0], scenario.boardCards.flopCards[1],
                                            scenario.boardCards.flopCards[2]);
                logTestMessage("Override flop cards to: " + scenario.boardCards.flopCards[0].toString() + " " +
                               scenario.boardCards.flopCards[1].toString() + " " +
                               scenario.boardCards.flopCards[2].toString());
            }
            else if (currentState == GameState::Turn)
            {
                // Set flop + turn cards (4 cards)
                predeterminedBoard.dealFlop(scenario.boardCards.flopCards[0], scenario.boardCards.flopCards[1],
                                            scenario.boardCards.flopCards[2]);
                predeterminedBoard.dealTurn(scenario.boardCards.turnCard);
                logTestMessage("Override turn card to: " + scenario.boardCards.turnCard.toString());
            }
            else if (currentState == GameState::River)
            {
                // Set all 5 cards
                predeterminedBoard.dealFlop(scenario.boardCards.flopCards[0], scenario.boardCards.flopCards[1],
                                            scenario.boardCards.flopCards[2]);
                predeterminedBoard.dealTurn(scenario.boardCards.turnCard);
                predeterminedBoard.dealRiver(scenario.boardCards.riverCard);
                logTestMessage("Override river card to: " + scenario.boardCards.riverCard.toString());
            }

            myHand->getBoard().setBoardCards(predeterminedBoard);
        };

        logTestMessage("Predetermined card override events registered");
    }

    // Generic method to run any test scenario
    void runTestScenario(const TestScenario& scenario)
    {
        getLogger().info("=== Running Scenario: " + scenario.scenarioName + " ===");

        setupPlayersFromScenario(scenario);

        // Create board and hand
        myBoard = myFactory->createBoard(scenario.dealerPlayerId);
        myBoard->setSeatsList(mySeatsList);
        myBoard->setActingPlayersList(myActingPlayersList);

        StartData startData;
        startData.startDealerPlayerId = scenario.dealerPlayerId;
        startData.numberOfPlayers = static_cast<int>(scenario.players.size());

        myHand = myFactory->createHand(myFactory, myBoard, mySeatsList, myActingPlayersList, gameData, startData);

        setupPredeterminedCardsFromScenario();

        myHand->initialize();

        EXPECT_EQ(myHand->getGameState(), Preflop);

        myHand->runGameLoop();

        logPlayerActions();

        EXPECT_EQ(myHand->getGameState(), PostRiver);

        // Log final results
        getLogger().info("=== Scenario " + scenario.scenarioName + " Completed ===");
        getLogger().info("");
    }

    void logPlayerActions()
    {
        for (const auto& player : *mySeatsList)
        {
            auto lastAction = player->getLastAction();
            getLogger().info("Player " + std::to_string(player->getId()) + " (" + player->getName() + ") " +
                             "Cards: " + player->getHoleCards().toString() + " Last Action: " +
                             actionTypeToString(lastAction.type) + " Amount: " + std::to_string(lastAction.amount));
        }
    }

    void logPlayersHoleCardsAfterDealing()
    {
        getLogger().info("=== Players' Hole Cards After Override ===");
        for (const auto& player : *mySeatsList)
        {
            const HoleCards& holeCards = player->getHoleCards();
            if (holeCards.isValid())
            {
                getLogger().info("Player " + std::to_string(player->getId()) + " (" + player->getName() +
                                 "): " + holeCards.toString());
            }
            else
            {
                getLogger().info("Player " + std::to_string(player->getId()) + " (" + player->getName() +
                                 "): No valid hole cards");
            }
        }
    }

    void logTestMessage(const std::string& message) const { getLogger().info("StrategiesE2ETest: " + message); }
};

// Predefined scenarios for easy reuse
namespace TestScenarios
{
TestScenario FourStrategiesBasic()
{
    std::vector<PlayerConfig> players = {
        PlayerConfig(0, "TightAggressive", StrategyType::TightAggressive, HoleCards("As", "Ah"), 1000),
        PlayerConfig(1, "LooseAggressive", StrategyType::LooseAggressive, HoleCards("9s", "8s"), 1000),
        PlayerConfig(2, "UltraTight", StrategyType::UltraTight, HoleCards("7h", "7c"), 1000),
        PlayerConfig(3, "Maniac", StrategyType::Maniac, HoleCards("Kd", "3c"), 1000)};

    BoardConfig board("9h", "8h", "7d", "As", "2c");

    return TestScenario("Four Strategies Basic", players, board, 0, 10,
                        "Test all four bot strategies with varied starting hands");
}

TestScenario IdenticalHandsScenario()
{
    std::vector<PlayerConfig> players = {
        PlayerConfig(0, "TightAggressive", StrategyType::TightAggressive, HoleCards("Qd", "Js"), 1000),
        PlayerConfig(1, "LooseAggressive", StrategyType::LooseAggressive, HoleCards("Qd", "Js"), 1000),
        PlayerConfig(2, "UltraTight", StrategyType::UltraTight, HoleCards("Qd", "Js"), 1000),
        PlayerConfig(3, "Maniac", StrategyType::Maniac, HoleCards("Qd", "Js"), 1000)};

    BoardConfig board("Ts", "9c", "8h", "7s", "2d");

    return TestScenario("Identical Hands", players, board, 0, 10,
                        "All players have same hand - test strategy differences");
}

// Weak hands scenario to emphasize strategy differences
TestScenario WeakHandsScenario()
{
    std::vector<PlayerConfig> players = {
        PlayerConfig(0, "TightAggressive", StrategyType::TightAggressive, HoleCards("2c", "7d"), 1000),
        PlayerConfig(1, "LooseAggressive", StrategyType::LooseAggressive, HoleCards("5s", "6s"), 1000),
        PlayerConfig(2, "UltraTight", StrategyType::UltraTight, HoleCards("9c", "4d"), 1000),
        PlayerConfig(3, "Maniac", StrategyType::Maniac, HoleCards("Jd", "3h"), 1000)};

    BoardConfig board("Ah", "8c", "5d", "2s", "Qh");

    return TestScenario("Weak Hands", players, board, 0, 10,
                        "Weak hands on rainbow board - test tight vs loose strategies");
}

TestScenario ThreePlayerScenario()
{
    std::vector<PlayerConfig> players = {
        PlayerConfig(0, "TightAggressive", StrategyType::TightAggressive, HoleCards("Kh", "Kd"), 1000),
        PlayerConfig(1, "LooseAggressive", StrategyType::LooseAggressive, HoleCards("Ac", "Qc"), 1000),
        PlayerConfig(2, "Maniac", StrategyType::Maniac, HoleCards("2h", "7s"), 1000)};

    BoardConfig board("Jc", "Ts", "9h", "Kc", "Ad");

    return TestScenario("Three Players", players, board, 0, 10, "Three-player hand with strong starting hands");
}

// === ALL-IN SCENARIOS ===

TestScenario ShortStackAllInPreflop()
{
    std::vector<PlayerConfig> players = {
        PlayerConfig(0, "TightAggressive", StrategyType::TightAggressive, HoleCards("As", "Ah"), 1000),
        PlayerConfig(1, "LooseAggressive", StrategyType::LooseAggressive, HoleCards("Kh", "Kd"), 1000),
        PlayerConfig(2, "UltraTight", StrategyType::UltraTight, HoleCards("Qc", "Qd"), 50), // Short stack
        PlayerConfig(3, "Maniac", StrategyType::Maniac, HoleCards("Jh", "Js"), 1000)};

    BoardConfig board("9h", "8c", "7d", "2s", "As");

    return TestScenario("Short Stack All-In Preflop", players, board, 0, 10,
                        "Test all-in with short stack preflop - validates pot calculation");
}

TestScenario MidHandAllIn()
{
    std::vector<PlayerConfig> players = {
        PlayerConfig(0, "TightAggressive", StrategyType::TightAggressive, HoleCards("Ah", "Kh"), 1000),
        PlayerConfig(1, "LooseAggressive", StrategyType::LooseAggressive, HoleCards("9s", "8s"), 200), // Medium stack
        PlayerConfig(2, "UltraTight", StrategyType::UltraTight, HoleCards("7c", "6c"), 1000),
        PlayerConfig(3, "Maniac", StrategyType::Maniac, HoleCards("2d", "2h"), 1000)};

    BoardConfig board("9h", "8h", "7d", "Kc", "2c");

    return TestScenario("Mid-Hand All-In", players, board, 0, 10,
                        "Test all-in during flop/turn - validates betting round continuation");
}

TestScenario MultipleAllIns()
{
    std::vector<PlayerConfig> players = {
        PlayerConfig(0, "TightAggressive", StrategyType::TightAggressive, HoleCards("As", "Ad"), 300),
        PlayerConfig(1, "LooseAggressive", StrategyType::LooseAggressive, HoleCards("Kh", "Kd"), 150),
        PlayerConfig(2, "UltraTight", StrategyType::UltraTight, HoleCards("Qc", "Qd"), 75),
        PlayerConfig(3, "Maniac", StrategyType::Maniac, HoleCards("Jh", "Js"), 1000)};

    BoardConfig board("9h", "8c", "7d", "6s", "5c");

    return TestScenario("Multiple All-Ins", players, board, 0, 10,
                        "Test multiple all-ins with different stack sizes - validates side pot creation");
}

// === HEADS-UP SCENARIOS ===

TestScenario HeadsUpTightVsAggressive()
{
    std::vector<PlayerConfig> players = {
        PlayerConfig(0, "TightAggressive", StrategyType::TightAggressive, HoleCards("As", "Kd"), 1000),
        PlayerConfig(1, "LooseAggressive", StrategyType::LooseAggressive, HoleCards("Tc", "9c"), 1000)};

    BoardConfig board("Kh", "9s", "7d", "2c", "Ah");

    return TestScenario("Heads-Up Tight vs Aggressive", players, board, 0, 10,
                        "Test two-player dynamics - validates minimal player count handling");
}

TestScenario HeadsUpPositionTest()
{
    std::vector<PlayerConfig> players = {
        PlayerConfig(0, "UltraTight", StrategyType::UltraTight, HoleCards("Qh", "Jh"), 1000), // Button/Small blind
        PlayerConfig(1, "Maniac", StrategyType::Maniac, HoleCards("8d", "7s"), 1000)};        // Big blind

    BoardConfig board("Jc", "8h", "6d", "3s", "Qc");

    return TestScenario("Heads-Up Position Test", players, board, 0, 10,
                        "Test button vs big blind dynamics in heads-up play");
}

TestScenario HeadsUpShortStacks()
{
    std::vector<PlayerConfig> players = {
        PlayerConfig(0, "TightAggressive", StrategyType::TightAggressive, HoleCards("Ad", "Kc"), 80),
        PlayerConfig(1, "LooseAggressive", StrategyType::LooseAggressive, HoleCards("Th", "9h"), 120)};

    BoardConfig board("Kd", "Ts", "4h", "2c", "As");

    return TestScenario("Heads-Up Short Stacks", players, board, 0, 10,
                        "Test heads-up with short stacks - validates push/fold scenarios");
}
} // namespace TestScenarios

TEST_F(StrategiesE2ETest, FourStrategiesCompleteHandFromPreflopToPostRiver)
{
    runTestScenario(TestScenarios::FourStrategiesBasic());
}

TEST_F(StrategiesE2ETest, StrategiesBehaveDifferentlyInSameScenario)
{
    runTestScenario(TestScenarios::IdenticalHandsScenario());
}

TEST_F(StrategiesE2ETest, ManiacVsTightStrategiesShowClearDifferences)
{
    runTestScenario(TestScenarios::WeakHandsScenario());
}

TEST_F(StrategiesE2ETest, ThreePlayerScenario)
{
    runTestScenario(TestScenarios::ThreePlayerScenario());
}

// === ALL-IN SCENARIO TESTS ===

TEST_F(StrategiesE2ETest, ShortStackAllInPreflopValidatesPotCalculation)
{
    runTestScenario(TestScenarios::ShortStackAllInPreflop());
}

TEST_F(StrategiesE2ETest, MidHandAllInValidatesBettingContinuation)
{
    runTestScenario(TestScenarios::MidHandAllIn());
}

TEST_F(StrategiesE2ETest, MultipleAllInsValidateSidePotCreation)
{
    runTestScenario(TestScenarios::MultipleAllIns());
}

// === HEADS-UP SCENARIO TESTS ===

TEST_F(StrategiesE2ETest, HeadsUpValidatesMinimalPlayerCount)
{
    runTestScenario(TestScenarios::HeadsUpTightVsAggressive());
}

TEST_F(StrategiesE2ETest, HeadsUpPositionDynamicsValidation)
{
    runTestScenario(TestScenarios::HeadsUpPositionTest());
}

TEST_F(StrategiesE2ETest, HeadsUpShortStacksPushFoldValidation)
{
    runTestScenario(TestScenarios::HeadsUpShortStacks());
}

} // namespace pkt::test