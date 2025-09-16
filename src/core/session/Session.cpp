// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include <core/engine/Game.h>
#include <core/engine/GameEvents.h>
#include "Session.h"

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

Session::Session(const GameEvents& events) : myEvents(events), myEngineFactory(nullptr)
{
}

Session::Session(const GameEvents& events, std::shared_ptr<EngineFactory> engineFactory)
    : myEvents(events), myEngineFactory(engineFactory)
{
}

Session::~Session() = default;

pkt::core::player::PlayerList Session::createPlayersList(DefaultPlayerFactory& playerFactory, int numberOfPlayers,
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

std::unique_ptr<player::DefaultPlayerFactory> Session::createPlayerFactory(const GameEvents& events,
                                                                           player::StrategyAssigner* strategyAssigner)
{
    return std::make_unique<player::DefaultPlayerFactory>(events, strategyAssigner);
}

std::shared_ptr<IBoard> Session::createBoard(const StartData& startData)
{
    if (!myEngineFactory)
        throw std::runtime_error("EngineFactory not initialized");

    auto board = myEngineFactory->createBoard(startData.startDealerPlayerId);
    return board;
}

void Session::startGame(const GameData& gameData, const StartData& startData)
{
    validateGameParameters(gameData, startData);
    ensureEngineFactoryInitialized();
    auto gameComponents = createGameComponents(gameData, startData);
    initializeGame(std::move(gameComponents), gameData, startData);
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
    if (myEvents.onGameInitialized)
    {
        myEvents.onGameInitialized(guiSpeed);
    }
}

void Session::ensureEngineFactoryInitialized()
{
    if (!myEngineFactory)
    {
        myEngineFactory = std::make_shared<EngineFactory>(myEvents);
    }
}

Session::GameComponents Session::createGameComponents(const GameData& gameData, const StartData& startData)
{
    GameComponents components;

    // Create dependencies using virtual factory methods (testable)
    components.strategyAssigner = createStrategyAssigner(gameData.tableProfile, startData.numberOfPlayers - 1);
    components.playerFactory = createPlayerFactory(myEvents, components.strategyAssigner.get());

    components.playersList = createPlayersList(*components.playerFactory, startData.numberOfPlayers,
                                               gameData.startMoney, gameData.tableProfile);

    // Create board using factory method (testable)
    components.board = createBoard(startData);
    components.board->setSeatsList(components.playersList);
    components.board->setActingPlayersList(components.playersList);

    return components;
}

void Session::initializeGame(GameComponents&& components, const GameData& gameData, const StartData& startData)
{
    myCurrentGame = std::make_unique<Game>(myEvents, myEngineFactory, components.board, components.playersList,
                                           startData.startDealerPlayerId, gameData, startData);
    fireGameInitializedEvent(gameData.guiSpeed);
    myCurrentGame->startNewHand();
}

} // namespace pkt::core
