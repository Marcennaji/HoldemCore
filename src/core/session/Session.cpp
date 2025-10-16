// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Session.h"
#include <core/engine/GameEvents.h>
#include <core/engine/game/Game.h>

#include <core/engine/EngineFactory.h>
#include <core/player/strategy/LooseAggressiveBotStrategy.h>
#include <core/player/strategy/ManiacBotStrategy.h>
#include <core/player/strategy/TightAggressiveBotStrategy.h>
#include <core/player/strategy/UltraTightBotStrategy.h>
#include "core/player/PlayerFactory.h"
#include "core/player/strategy/HumanStrategy.h"
#include "core/player/strategy/StrategyAssigner.h"

#include <algorithm>
#include <random>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace pkt::core
{

using namespace std;
using namespace pkt::core::player;

Session::Session(const GameEvents& events, EngineFactory& engineFactory, Logger& logger,
                 HandEvaluationEngine& handEvaluationEngine, PlayersStatisticsStore& playersStatisticsStore,
                 Randomizer& randomizer)
    : m_events(events), m_engineFactory(engineFactory), m_logger(logger), m_handEvaluationEngine(handEvaluationEngine),
      m_playersStatisticsStore(playersStatisticsStore), m_randomizer(randomizer)
{
}

Session::~Session() = default;

pkt::core::player::PlayerList Session::createPlayersList(PlayerFactory& playerFactory, int numberOfPlayers,
                                                         unsigned startMoney, const TableProfile& tableProfile)
{
    auto playersList = std::make_shared<std::list<std::shared_ptr<Player>>>();

    for (int i = 0; i < numberOfPlayers; ++i)
        playersList->push_back(playerFactory.createPlayer(i, tableProfile, startMoney));

    return playersList;
}

std::unique_ptr<player::StrategyAssigner> Session::createStrategyAssigner(const TableProfile& tableProfile,
                                                                          int numberOfBots)
{
    return std::make_unique<player::StrategyAssigner>(tableProfile, numberOfBots, m_logger, m_randomizer);
}

std::unique_ptr<player::PlayerFactory> Session::createPlayerFactory(const GameEvents& events,
                                                                    player::StrategyAssigner* strategyAssigner)
{
    return std::make_unique<player::PlayerFactory>(events, strategyAssigner, m_logger, m_handEvaluationEngine,
                                                   m_playersStatisticsStore, m_randomizer);
}

std::shared_ptr<Board> Session::createBoard(const StartData& startData)
{
    auto board = m_engineFactory.createBoard(startData.startDealerPlayerId);
    return board;
}

void Session::startGame(const GameData& gameData, const StartData& startData)
{
    validateGameParameters(gameData, startData);

    // Auto-select dealer if not specified
    StartData adjustedStartData = startData;
    if (adjustedStartData.startDealerPlayerId == StartData::AUTO_SELECT_DEALER)
    {
        int randomDealer = 0;
        m_randomizer.getRand(0, adjustedStartData.numberOfPlayers - 1, 1, &randomDealer);
        adjustedStartData.startDealerPlayerId = randomDealer;
    }

    auto gameComponents = createGameComponents(gameData, adjustedStartData);
    initializeGame(std::move(gameComponents), gameData, adjustedStartData);
}

void Session::startBotOnlyGameWithCustomStrategies(const BotGameData& botGameData, const StartData& startData)
{
    // Validate strategy distribution
    int totalPlayers = 0;
    for (const auto& [strategyName, count] : botGameData.strategyDistribution)
    {
        if (count <= 0)
        {
            throw std::runtime_error("Strategy count must be positive for strategy: " + strategyName);
        }
        totalPlayers += count;
    }

    if (totalPlayers == 0)
    {
        throw std::runtime_error("Total player count must be positive");
    }

    if (totalPlayers != startData.numberOfPlayers)
    {
        throw std::runtime_error("Strategy distribution total (" + std::to_string(totalPlayers) +
                                 ") must match numberOfPlayers (" + std::to_string(startData.numberOfPlayers) + ")");
    }

    // Auto-select dealer if needed
    StartData adjustedStartData = startData;
    if (adjustedStartData.startDealerPlayerId == StartData::AUTO_SELECT_DEALER)
    {
        int randomDealer = 0;
        m_randomizer.getRand(0, adjustedStartData.numberOfPlayers - 1, 1, &randomDealer);
        adjustedStartData.startDealerPlayerId = randomDealer;
    }

    // Convert BotGameData to GameData
    GameData gameData;
    gameData.startMoney = botGameData.startMoney;
    gameData.firstSmallBlind = botGameData.firstSmallBlind;
    gameData.guiSpeed = 0;                                 // Bot-only games are headless (no GUI)
    gameData.tableProfile = TableProfile::RandomOpponents; // Not used for custom strategies
    gameData.maxNumberOfPlayers = totalPlayers;

    // Create custom components
    auto components = createCustomStrategyComponents(botGameData, adjustedStartData);

    // Initialize session with custom components
    initializeGame(std::move(components), gameData, adjustedStartData);
}

void Session::validatePlayerConfiguration(const pkt::core::player::PlayerList& playersList)
{
    if (!playersList || playersList->empty())
    {
        throw std::runtime_error("Player list cannot be empty");
    }

    // Validate that all players are valid (not null)
    for (const auto& player : *playersList)
    {
        if (!player)
        {
            throw std::runtime_error("Invalid null player in player list");
        }
    }

    // Validate exactly 1 human player with ID 0
    int humanPlayerCount = 0;
    bool hasHumanPlayerWithIdZero = false;
    std::vector<int> humanPlayerIds;

    for (const auto& player : *playersList)
    {
        // Check if this player has a HumanStrategy
        if (player->hasStrategyType<player::HumanStrategy>())
        {
            humanPlayerCount++;
            humanPlayerIds.push_back(player->getId());
            if (player->getId() == 0)
            {
                hasHumanPlayerWithIdZero = true;
            }
        }
    }

    // Validate exactly 1 human player
    if (humanPlayerCount != 1)
    {
        std::string errorMsg = "Game must have exactly 1 human player, found " + std::to_string(humanPlayerCount);
        if (!humanPlayerIds.empty())
        {
            errorMsg += " (Human player IDs: ";
            for (size_t i = 0; i < humanPlayerIds.size(); ++i)
            {
                if (i > 0)
                    errorMsg += ", ";
                errorMsg += std::to_string(humanPlayerIds[i]);
            }
            errorMsg += ")";
        }
        throw std::runtime_error(errorMsg);
    }

    // Validate human player has ID 0
    if (!hasHumanPlayerWithIdZero)
    {
        throw std::runtime_error("Human player must have ID 0, but found human player with ID " +
                                 std::to_string(humanPlayerIds[0]));
    }
}

void Session::validateGameParameters(const GameData& gameData, const StartData& startData)
{
    if (startData.numberOfPlayers < 2)
    {
        throw std::invalid_argument("Game requires at least 2 players");
    }
    if (startData.numberOfPlayers > 10)
    {
        throw std::invalid_argument("Game supports maximum 10 players");
    }
    if (gameData.startMoney <= 0)
    {
        throw std::invalid_argument("Start money must be greater than 0");
    }
    // Allow AUTO_SELECT_DEALER (-1), otherwise validate the dealer ID is a valid player index
    if (startData.startDealerPlayerId != StartData::AUTO_SELECT_DEALER &&
        (startData.startDealerPlayerId < 0 || startData.startDealerPlayerId >= startData.numberOfPlayers))
    {
        throw std::invalid_argument("Dealer player ID must be valid player index");
    }
}

void Session::fireGameInitializedEvent(int guiSpeed)
{
    if (m_events.onGameInitialized)
    {
        m_events.onGameInitialized(guiSpeed);
    }
}

Session::GameComponents Session::createGameComponents(const GameData& gameData, const StartData& startData)
{
    GameComponents components;

    // Create dependencies using virtual factory methods (testable)
    int numberOfBots = startData.numberOfPlayers - 1; // Always 1 human + (n-1) bots
    components.strategyAssigner = createStrategyAssigner(gameData.tableProfile, numberOfBots);
    components.playerFactory = createPlayerFactory(m_events, components.strategyAssigner.get());

    components.playersList = createPlayersList(*components.playerFactory, startData.numberOfPlayers,
                                               gameData.startMoney, gameData.tableProfile);

    // Validate that we have exactly 1 human player with ID 0
    validatePlayerConfiguration(components.playersList);

    // Create board using factory method (testable)
    components.board = createBoard(startData);
    components.board->setSeatsList(components.playersList);
    components.board->setActingPlayersList(components.playersList);

    return components;
}

void Session::initializeGame(GameComponents&& components, const GameData& gameData, const StartData& startData)
{
    // Fire event with player list before creating the game
    if (m_events.onPlayersInitialized)
    {
        m_events.onPlayersInitialized(components.playersList);
    }

    m_currentGame = std::make_unique<Game>(m_events, m_engineFactory, components.board, components.playersList,
                                           startData.startDealerPlayerId, gameData, startData);
    fireGameInitializedEvent(gameData.guiSpeed);
    m_currentGame->startNewHand();
}

void Session::handlePlayerAction(const PlayerAction& action)
{
    if (m_currentGame)
    {
        m_currentGame->handlePlayerAction(action);
    }
}

void Session::startNewHand()
{
    if (m_currentGame)
    {
        m_currentGame->startNewHand();
    }
}

Session::GameComponents Session::createCustomStrategyComponents(const BotGameData& botGameData,
                                                                const StartData& startData)
{
    GameComponents components;

    // We don't use StrategyAssigner for custom strategies - create players manually
    components.strategyAssigner = nullptr;

    // Create a player factory (without strategy assigner since we'll set strategies manually)
    components.playerFactory = createPlayerFactory(m_events, nullptr);

    // Create players list
    components.playersList = std::make_shared<std::list<std::shared_ptr<player::Player>>>();

    // Create players with specific strategies based on distribution map
    int playerId = 0;
    for (const auto& [strategyName, count] : botGameData.strategyDistribution)
    {
        for (int i = 0; i < count; ++i)
        {
            // Create bot player manually
            auto player = std::make_shared<player::Player>(m_events, m_logger, m_handEvaluationEngine,
                                                           m_playersStatisticsStore, m_randomizer, playerId,
                                                           "Bot_" + std::to_string(playerId), botGameData.startMoney);
            playerId++;

            // Create the appropriate strategy based on name
            std::unique_ptr<player::BotStrategy> strategy;

            std::string lowerStrategyName = strategyName;
            std::transform(lowerStrategyName.begin(), lowerStrategyName.end(), lowerStrategyName.begin(), ::tolower);

            if (lowerStrategyName == "tight")
            {
                strategy = std::make_unique<player::TightAggressiveBotStrategy>(m_logger, m_randomizer);
            }
            else if (lowerStrategyName == "loose")
            {
                strategy = std::make_unique<player::LooseAggressiveBotStrategy>(m_logger, m_randomizer);
            }
            else if (lowerStrategyName == "maniac")
            {
                strategy = std::make_unique<player::ManiacBotStrategy>(m_logger, m_randomizer);
            }
            else if (lowerStrategyName == "ultratight")
            {
                strategy = std::make_unique<player::UltraTightBotStrategy>(m_logger, m_randomizer);
            }
            else
            {
                throw std::runtime_error("Unknown strategy name: " + strategyName);
            }

            // Set the strategy on the player
            player->setStrategy(std::move(strategy));

            // Add to players list
            components.playersList->push_back(player);
        }
    }

    // Create board
    components.board = createBoard(startData);
    components.board->setSeatsList(components.playersList);

    // Create a copy for acting players list
    auto actingPlayersList = std::make_shared<std::list<std::shared_ptr<player::Player>>>(*components.playersList);
    components.board->setActingPlayersList(actingPlayersList);

    return components;
}

} // namespace pkt::core
