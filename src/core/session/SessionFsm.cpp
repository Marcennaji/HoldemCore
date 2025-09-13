// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "SessionFsm.h"
#include <core/engine/GameEvents.h>
#include <core/engine/GameFsm.h>

#include <core/engine/EngineFactory.h>

#include <core/player/strategy/LooseAggressiveBotStrategy.h>
#include <core/player/strategy/ManiacBotStrategy.h>
#include <core/player/strategy/TightAggressiveBotStrategy.h>
#include <core/player/strategy/UltraTightBotStrategy.h>
#include <core/services/GlobalServices.h>
#include "core/player/DefaultPlayerFactory.h"
#include "core/player/strategy/StrategyAssigner.h"

#include <algorithm>
#include <random>
#include <sstream>
#include <stdexcept>

namespace pkt::core
{

using namespace std;
using namespace pkt::core::player;

SessionFsm::SessionFsm(const GameEvents& events) : myEvents(events), myEngineFactory(nullptr)
{
}

SessionFsm::SessionFsm(const GameEvents& events, std::shared_ptr<EngineFactory> engineFactory)
    : myEvents(events), myEngineFactory(engineFactory)
{
}

SessionFsm::~SessionFsm() = default;

pkt::core::player::PlayerFsmList SessionFsm::createPlayersList(DefaultPlayerFactory& playerFactory, int numberOfPlayers,
                                                               unsigned startMoney, const TableProfile& tableProfile)
{
    auto playersList = std::make_shared<std::list<std::shared_ptr<PlayerFsm>>>();

    for (int i = 0; i < numberOfPlayers; ++i)
        playersList->push_back(playerFactory.createPlayerFsm(i, tableProfile, startMoney));

    return playersList;
}

std::unique_ptr<player::StrategyAssigner> SessionFsm::createStrategyAssigner(const TableProfile& tableProfile,
                                                                             int numberOfBots)
{
    return std::make_unique<player::StrategyAssigner>(tableProfile, numberOfBots);
}

std::unique_ptr<player::DefaultPlayerFactory>
SessionFsm::createPlayerFactory(const GameEvents& events, player::StrategyAssigner* strategyAssigner)
{
    return std::make_unique<player::DefaultPlayerFactory>(events, strategyAssigner);
}

std::shared_ptr<IBoard> SessionFsm::createBoard(const StartData& startData)
{
    if (!myEngineFactory)
        throw std::runtime_error("EngineFactory not initialized");

    auto board = myEngineFactory->createBoardFsm(startData.startDealerPlayerId);
    return board;
}

void SessionFsm::startGame(const GameData& gameData, const StartData& startData)
{
    // Validate input parameters
    validateGameParameters(gameData, startData);

    // Fire initialization event
    fireGameInitializedEvent(gameData.guiSpeed);

    // Ensure engine factory is available
    ensureEngineFactoryInitialized();

    // Create game components
    auto gameComponents = createGameComponents(gameData, startData);

    // Initialize and start the game
    initializeGame(std::move(gameComponents), gameData, startData);
}

void SessionFsm::validateGameParameters(const GameData& gameData, const StartData& startData)
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

void SessionFsm::fireGameInitializedEvent(int guiSpeed)
{
    if (myEvents.onGameInitialized)
    {
        myEvents.onGameInitialized(guiSpeed);
    }
}

void SessionFsm::ensureEngineFactoryInitialized()
{
    if (!myEngineFactory)
    {
        myEngineFactory = std::make_shared<EngineFactory>(myEvents);
    }
}

SessionFsm::GameComponents SessionFsm::createGameComponents(const GameData& gameData, const StartData& startData)
{
    GameComponents components;

    // Create dependencies using virtual factory methods (testable)
    components.strategyAssigner = createStrategyAssigner(gameData.tableProfile, startData.numberOfPlayers - 1);
    components.playerFactory = createPlayerFactory(myEvents, components.strategyAssigner.get());

    components.playersList = createPlayersList(*components.playerFactory, startData.numberOfPlayers,
                                               gameData.startMoney, gameData.tableProfile);

    // Create board using factory method (testable)
    components.board = createBoard(startData);
    components.board->setSeatsListFsm(components.playersList);
    components.board->setActingPlayersListFsm(components.playersList);

    return components;
}

void SessionFsm::initializeGame(GameComponents&& components, const GameData& gameData, const StartData& startData)
{
    myCurrentGame = std::make_unique<GameFsm>(myEvents, myEngineFactory, components.board, components.playersList,
                                              startData.startDealerPlayerId, gameData, startData);

    myCurrentGame->startNewHand();
}

} // namespace pkt::core
