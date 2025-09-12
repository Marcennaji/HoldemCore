// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "SessionFsm.h"
#include <core/engine/GameEvents.h>
#include <core/engine/GameFsm.h>

#include <core/engine/EngineFactory.h>

#include <core/player/deprecated/BotPlayer.h>
#include <core/player/strategy/LooseAggressiveBotStrategy.h>
#include <core/player/strategy/ManiacBotStrategy.h>
#include <core/player/strategy/TightAggressiveBotStrategy.h>
#include <core/player/strategy/UltraTightBotStrategy.h>
#include <core/services/GlobalServices.h>
#include "core/engine/deprecated/Helpers.h"
#include "core/player/DefaultPlayerFactory.h"
#include "core/player/deprecated/HumanPlayer.h"
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
    myCurrentGame.reset();

    if (myEvents.onGameInitialized)
        myEvents.onGameInitialized(gameData.guiSpeed);

    // Create or use injected engine factory
    if (!myEngineFactory)
        myEngineFactory = std::make_shared<EngineFactory>(myEvents);

    // Create dependencies using virtual factory methods (testable)
    auto strategyAssigner = createStrategyAssigner(gameData.tableProfile, startData.numberOfPlayers - 1);
    auto playerFactory = createPlayerFactory(myEvents, strategyAssigner.get());

    auto playersList =
        createPlayersList(*playerFactory, startData.numberOfPlayers, gameData.startMoney, gameData.tableProfile);

    // Create board using factory method (testable)
    auto board = createBoard(startData);
    board->setSeatsListFsm(playersList);
    board->setActingPlayersListFsm(playersList);

    myCurrentGame = std::make_unique<GameFsm>(myEvents, myEngineFactory, board, playersList,
                                              startData.startDealerPlayerId, gameData, startData);

    myCurrentGame->startNewHand();
}

} // namespace pkt::core
