// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Session.h"
#include <core/engine/Game.h>
#include <core/engine/GameEvents.h>

#include <core/engine/EngineFactory.h>

#include <core/player/deprecated/BotPlayer.h>
#include <core/player/strategy/LooseAggressiveBotStrategy.h>
#include <core/player/strategy/ManiacBotStrategy.h>
#include <core/player/strategy/TightAggressiveBotStrategy.h>
#include <core/player/strategy/UltraTightBotStrategy.h>
#include <core/services/GlobalServices.h>
#include "core/player/DefaultPlayerFactory.h"
#include "core/player/Helpers.h"
#include "core/player/deprecated/HumanPlayer.h"
#include "core/player/strategy/StrategyAssigner.h"

#include <algorithm>
#include <random>
#include <sstream>

namespace pkt::core
{

using namespace std;
using namespace pkt::core::player;

Session::Session(const GameEvents& events) : myEvents(events)
{
}

Session::~Session() = default;

pkt::core::player::PlayerList Session::createPlayersList(DefaultPlayerFactory& playerFactory, int numberOfPlayers,
                                                         unsigned startMoney, const TableProfile& tableProfile)
{
    auto playersList = std::make_shared<std::list<std::shared_ptr<Player>>>();

    playersList->push_back(playerFactory.createHumanPlayer(0, startMoney));

    for (int i = 1; i < numberOfPlayers; ++i)
        playersList->push_back(playerFactory.createBotPlayer(i, tableProfile, startMoney));

    // Shuffle bots but keep human first
    shufflePlayers(*playersList, 0);

    // Reassign IDs after shuffle
    int id = 0;
    for (auto& p : *playersList)
        p->setId(id++);

    return playersList;
}

void Session::startGame(const GameData& gameData, const StartData& startData)
{
    myCurrentGame.reset();

    if (myEvents.onGameInitialized)
        myEvents.onGameInitialized(gameData.guiSpeed);

    auto engineFactory = std::make_shared<EngineFactory>(myEvents);
    auto strategyAssigner = std::make_unique<StrategyAssigner>(gameData.tableProfile, startData.numberOfPlayers - 1);
    auto playerFactory = std::make_unique<DefaultPlayerFactory>(myEvents, strategyAssigner.get());

    auto playersList =
        createPlayersList(*playerFactory, startData.numberOfPlayers, gameData.startMoney, gameData.tableProfile);

    // Board is fully prepared here
    auto board = engineFactory->createBoard(startData.startDealerPlayerId);
    board->setSeatsList(playersList);
    board->setRunningPlayersList(playersList);

    myCurrentGame = std::make_unique<Game>(myEvents, engineFactory, board, playersList, startData.startDealerPlayerId,
                                           gameData, startData);

    myCurrentGame->startNewHand();
}

} // namespace pkt::core
