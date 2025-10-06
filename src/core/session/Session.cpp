// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Session.h"
#include <core/engine/GameEvents.h>
#include <core/engine/game/Game.h>

#include <core/engine/EngineFactory.h>
#include <core/interfaces/ServiceAdapter.h>

#include <core/player/strategy/LooseAggressiveBotStrategy.h>
#include <core/player/strategy/ManiacBotStrategy.h>
#include <core/player/strategy/TightAggressiveBotStrategy.h>
#include <core/player/strategy/UltraTightBotStrategy.h>
#include <core/services/ServiceContainer.h>
#include "core/player/DefaultPlayerFactory.h"
#include "core/player/MixedPlayerFactory.h"
#include "core/player/strategy/StrategyAssigner.h"
#include "core/player/strategy/HumanStrategy.h"

#include <algorithm>
#include <random>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace pkt::core
{

using namespace std;
using namespace pkt::core::player;

Session::Session(const GameEvents& events) : m_events(events), m_engineFactory(nullptr), m_serviceContainer(nullptr)
{
}

Session::Session(const GameEvents& events, std::shared_ptr<EngineFactory> engineFactory)
    : m_events(events), m_engineFactory(engineFactory), m_serviceContainer(nullptr)
{
}

Session::Session(const GameEvents& events, std::shared_ptr<ServiceContainer> serviceContainer)
    : m_events(events), m_engineFactory(nullptr), m_serviceContainer(serviceContainer)
{
}

Session::~Session() = default;

pkt::core::player::PlayerList Session::createPlayersList(MixedPlayerFactory& playerFactory, int numberOfPlayers,
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
    return std::make_unique<player::StrategyAssigner>(tableProfile, numberOfBots);
}

std::unique_ptr<player::MixedPlayerFactory> Session::createPlayerFactory(const GameEvents& events,
                                                                           player::StrategyAssigner* strategyAssigner)
{
    if (m_serviceContainer) {
        // Use ISP-compliant constructor with service interfaces
        auto serviceAdapter = std::make_shared<ServiceAdapter>(m_serviceContainer);
        auto logger = serviceAdapter->createLoggerService();
        auto handEvaluator = serviceAdapter->createHandEvaluationEngineService();
        auto statisticsStore = serviceAdapter->createPlayersStatisticsStoreService();
        auto randomizer = serviceAdapter->createRandomizerService();
        
        return std::make_unique<player::MixedPlayerFactory>(events, strategyAssigner, logger, handEvaluator, statisticsStore, randomizer);
    } else {
        // Fallback to legacy constructor
        return std::make_unique<player::MixedPlayerFactory>(events, strategyAssigner);
    }
}

std::shared_ptr<Board> Session::createBoard(const StartData& startData)
{
    if (!m_engineFactory)
        throw std::runtime_error("EngineFactory not initialized");

    auto board = m_engineFactory->createBoard(startData.startDealerPlayerId);
    return board;
}

void Session::startGame(const GameData& gameData, const StartData& startData)
{
    validateGameParameters(gameData, startData);
    ensureEngineFactoryInitialized();
    ensureServiceContainerInitialized();
    auto gameComponents = createGameComponents(gameData, startData);
    initializeGame(std::move(gameComponents), gameData, startData);
}

void Session::validatePlayerConfiguration(const pkt::core::player::PlayerList& playersList)
{
    if (!playersList || playersList->empty()) {
        throw std::runtime_error("Player list cannot be empty");
    }
    
    int humanPlayerCount = 0;
    bool hasHumanPlayerWithIdZero = false;
    std::vector<int> humanPlayerIds;
    
    for (const auto& player : *playersList) {
        if (!player) {
            throw std::runtime_error("Invalid null player in player list");
        }
        
        // Check if this player has a HumanStrategy
        if (player->hasStrategyType<player::HumanStrategy>()) {
            humanPlayerCount++;
            humanPlayerIds.push_back(player->getId());
            if (player->getId() == 0) {
                hasHumanPlayerWithIdZero = true;
            }
        }
    }
    
    // Validate exactly 1 human player
    if (humanPlayerCount != 1) {
        std::string errorMsg = "Game must have exactly 1 human player, found " + 
                              std::to_string(humanPlayerCount);
        if (!humanPlayerIds.empty()) {
            errorMsg += " (Human player IDs: ";
            for (size_t i = 0; i < humanPlayerIds.size(); ++i) {
                if (i > 0) errorMsg += ", ";
                errorMsg += std::to_string(humanPlayerIds[i]);
            }
            errorMsg += ")";
        }
        throw std::runtime_error(errorMsg);
    }
    
    // Validate human player has ID 0
    if (!hasHumanPlayerWithIdZero) {
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
    if (gameData.startMoney == 0)
    {
        throw std::invalid_argument("Start money must be greater than 0");
    }
    if (startData.startDealerPlayerId >= startData.numberOfPlayers)
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

void Session::ensureEngineFactoryInitialized()
{
    if (!m_engineFactory)
    {
        m_engineFactory = std::make_shared<EngineFactory>(m_events);
    }
}

void Session::ensureServiceContainerInitialized()
{
    if (!m_serviceContainer)
    {
        m_serviceContainer = std::make_shared<AppServiceContainer>();
    }
}

Session::GameComponents Session::createGameComponents(const GameData& gameData, const StartData& startData)
{
    GameComponents components;

    // Create dependencies using virtual factory methods (testable)
    components.strategyAssigner = createStrategyAssigner(gameData.tableProfile, startData.numberOfPlayers - 1);
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
    m_currentGame = std::make_unique<Game>(m_events, m_engineFactory, components.board, components.playersList,
                                           startData.startDealerPlayerId, gameData, startData);
    fireGameInitializedEvent(gameData.guiSpeed);
    m_currentGame->startNewHand();
}

void Session::handlePlayerAction(const PlayerAction& action)
{
    if (m_currentGame) {
        m_currentGame->handlePlayerAction(action);
    }
}

void Session::startNewHand()
{
    if (m_currentGame) {
        m_currentGame->startNewHand();
    }
}

} // namespace pkt::core
